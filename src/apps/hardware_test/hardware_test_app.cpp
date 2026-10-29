// Hardware Test app: checks every module and shows PASS or FAIL.

#include "apps/hardware_test/hardware_test_app.h"

#include <Arduino.h>

#include "config.h"
#include "core/bus_manager.h"

namespace {

// Same order as CheckId
const char* const CHECK_NAMES[] = {"OLED", "PN532", "SD card", "CC1101", "NRF24"};

// Current CC1101 chips report 0x14, older ones 0x04.
constexpr uint8_t CC1101_VERSION_CURRENT = 0x14;
constexpr uint8_t CC1101_VERSION_OLD = 0x04;

}  // namespace

HardwareTestApp::HardwareTestApp(DisplayManager& display, StorageManager& storage)
    : display_(display), storage_(storage) {
    static_assert(sizeof(CHECK_NAMES) / sizeof(CHECK_NAMES[0]) == CheckCount,
                  "CHECK_NAMES must have one name per CheckId");
    for (size_t i = 0; i < CheckCount; i++) {
        results_[i].name = CHECK_NAMES[i];
        labels_[i] = results_[i].label;
    }
}

const char* HardwareTestApp::name() const {
    return "Hardware Test";
}

bool HardwareTestApp::onEnter() {
    display_.showMessage("Hardware Test", "Running checks...", "");
    Serial.println("[INFO] Hardware test: running checks");
    runChecks();

    list_.setItems(labels_, CheckCount);
    screen_ = Screen::Results;
    needsRedraw_ = true;
    return true;
}

// Nothing to release: every check finishes inside runChecks().
void HardwareTestApp::onExit() {}

void HardwareTestApp::onLoop() {
    if (!needsRedraw_) {
        return;
    }
    draw();
    needsRedraw_ = false;
}

bool HardwareTestApp::onInput(InputEvent event) {
    if (screen_ == Screen::Details) {
        screen_ = Screen::Results;  // any button returns to the list
        needsRedraw_ = true;
        return true;
    }
    return handleResultsInput(event);
}

void HardwareTestApp::runChecks() {
    BusManager::logI2cDevices();
    checkI2cDevice(Oled, OLED_I2C_ADDRESS, "Check SDA/SCL wires");
    checkI2cDevice(Pn532, PN532_I2C_ADDRESS, "Set switches to I2C");
    checkSdCard();
    checkCc1101();
    checkNrf24();
}

void HardwareTestApp::checkI2cDevice(CheckId id, uint8_t address, const char* hint) {
    bool present = BusManager::isI2cDevicePresent(address);
    char detail[LINE_LENGTH];
    if (present) {
        snprintf(detail, sizeof(detail), "Found at 0x%02X", address);
    } else {
        snprintf(detail, sizeof(detail), "No answer at 0x%02X", address);
    }
    setResult(id, present ? Status::Pass : Status::Fail, detail, hint);
}

void HardwareTestApp::checkSdCard() {
    // Mounting again also picks up a card inserted after boot.
    if (!storage_.begin()) {
        setResult(SdCard, Status::Fail, "Mount failed", "FAT32? CS/MISO wires?");
    } else if (!storage_.testReadWrite()) {
        setResult(SdCard, Status::Fail, "Read/write mismatch", "Wiring or bad card");
    } else {
        setResult(SdCard, Status::Pass, "Mounted, R/W OK", "");
    }
}

void HardwareTestApp::checkCc1101() {
    uint8_t version = 0;
    if (!cc1101_.readVersion(version)) {
        setResult(Cc1101, Status::Fail, "No answer (MISO high)", "Check CSN, MISO, 3V3");
        return;
    }
    char detail[LINE_LENGTH];
    snprintf(detail, sizeof(detail), "VERSION 0x%02X", version);
    bool known = version == CC1101_VERSION_CURRENT || version == CC1101_VERSION_OLD;
    setResult(Cc1101, known ? Status::Pass : Status::Fail, detail, "Check CSN, MISO, 3V3");
}

void HardwareTestApp::checkNrf24() {
    nrf24_.begin();
    uint8_t setupAw = 0;
    bool connected = nrf24_.isChipConnected(setupAw);
    char detail[LINE_LENGTH];
    snprintf(detail, sizeof(detail), "SETUP_AW 0x%02X", setupAw);
    setResult(Nrf24, connected ? Status::Pass : Status::Fail, detail, "Check CSN, CE, 3V3");
}

// The hint is shown and logged only when the check did not pass.
void HardwareTestApp::setResult(CheckId id, Status status, const char* detail, const char* hint) {
    CheckResult& result = results_[id];
    result.status = status;
    result.hint = hint;
    snprintf(result.detail, sizeof(result.detail), "%s", detail);
    snprintf(result.label, sizeof(result.label), "%-13s%s", result.name, statusText(status));
    needsRedraw_ = true;

    if (status == Status::Pass) {
        Serial.printf("[PASS] %s: %s\n", result.name, detail);
    } else {
        Serial.printf("[%s] %s: %s (%s)\n", statusText(status), result.name, detail, hint);
    }
}

bool HardwareTestApp::handleResultsInput(InputEvent event) {
    if (event == InputEvent::Back) {
        return false;
    }
    if (event == InputEvent::Ok) {
        screen_ = Screen::Details;
        needsRedraw_ = true;
    } else if (list_.handleInput(event)) {
        needsRedraw_ = true;
    }
    return true;
}

void HardwareTestApp::draw() {
    if (screen_ == Screen::Details) {
        drawDetails();
        return;
    }
    display_.clear();
    list_.draw(display_);
    display_.show();
}

void HardwareTestApp::drawDetails() {
    const CheckResult& result = results_[list_.selectedIndex()];
    char title[LINE_LENGTH];
    snprintf(title, sizeof(title), "%s: %s", result.name, statusText(result.status));
    const char* bottomLine = result.status == Status::Pass ? "Any button: back" : result.hint;
    display_.showMessage(title, result.detail, bottomLine);
}

const char* HardwareTestApp::statusText(Status status) {
    switch (status) {
        case Status::Pass:
            return "PASS";
        case Status::Fail:
            return "FAIL";
    }
    return "?";
}
