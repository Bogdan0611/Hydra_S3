// Hardware Test app: checks every module and shows PASS or FAIL.

#include "apps/hardware_test/hardware_test_app.h"

#include <Arduino.h>

#include "config.h"
#include "core/bus_manager.h"

namespace {

// Same order as CheckId
const char* const CHECK_NAMES[] = {"OLED",  "PN532",       "SD card", "CC1101",
                                   "NRF24", "IR receiver", "Buzzer"};

// Current CC1101 chips report 0x14, older ones 0x04.
constexpr uint8_t CC1101_VERSION_CURRENT = 0x14;
constexpr uint8_t CC1101_VERSION_OLD = 0x04;

// One remote key press produces dozens of edges; stray light only a few.
constexpr uint32_t IR_EDGES_FOR_PASS = 20;

}  // namespace

HardwareTestApp::HardwareTestApp(DisplayManager& display, StorageManager& storage, Buzzer& buzzer)
    : display_(display), storage_(storage), buzzer_(buzzer) {
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
    runAutomaticChecks();

    // These two need the user, so they stay WAIT until the user acts.
    startIrCheck();
    setResult(BuzzerTone, Status::Waiting, "Not tested yet", "OK plays a tone");

    list_.setItems(labels_, CheckCount);
    screen_ = Screen::Results;
    needsRedraw_ = true;
    return true;
}

void HardwareTestApp::onExit() {
    ir_.stopEdgeCount();
}

void HardwareTestApp::onLoop() {
    updateIrCheck();
    if (!needsRedraw_) {
        return;
    }
    draw();
    needsRedraw_ = false;
}

bool HardwareTestApp::onInput(InputEvent event) {
    switch (screen_) {
        case Screen::Results:
            return handleResultsInput(event);
        case Screen::Details:
            screen_ = Screen::Results;  // any button returns to the list
            break;
        case Screen::BuzzerQuestion:
            answerBuzzerQuestion(event);
            break;
    }
    needsRedraw_ = true;
    return true;
}

void HardwareTestApp::runAutomaticChecks() {
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

void HardwareTestApp::startIrCheck() {
    ir_.startEdgeCount();
    if (ir_.isIdle()) {
        setResult(IrReceiver, Status::Waiting, "Idle, no signal yet", "Press a remote key");
    } else {
        setResult(IrReceiver, Status::Fail, "Output stuck LOW", "Check VCC, GND, wire");
    }
}

void HardwareTestApp::updateIrCheck() {
    bool waiting = results_[IrReceiver].status == Status::Waiting;
    if (!waiting || ir_.edgeCount() < IR_EDGES_FOR_PASS) {
        return;
    }
    char detail[LINE_LENGTH];
    snprintf(detail, sizeof(detail), "%lu edges received", ir_.edgeCount());
    setResult(IrReceiver, Status::Pass, detail, "");
}

void HardwareTestApp::answerBuzzerQuestion(InputEvent event) {
    if (event == InputEvent::Ok) {
        setResult(BuzzerTone, Status::Pass, "Tone heard", "");
    } else if (event == InputEvent::Back) {
        setResult(BuzzerTone, Status::Fail, "No tone heard", "Check S wire, VCC");
    } else {
        buzzer_.success();  // any other button plays the tone again
        return;
    }
    screen_ = Screen::Results;
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
        // The buzzer row runs its check; every other row shows its details.
        if (list_.selectedIndex() == BuzzerTone) {
            buzzer_.success();
            screen_ = Screen::BuzzerQuestion;
        } else {
            screen_ = Screen::Details;
        }
        needsRedraw_ = true;
    } else if (list_.handleInput(event)) {
        needsRedraw_ = true;
    }
    return true;
}

void HardwareTestApp::draw() {
    switch (screen_) {
        case Screen::Results:
            display_.clear();
            list_.draw(display_);
            display_.show();
            break;
        case Screen::Details:
            drawDetails();
            break;
        case Screen::BuzzerQuestion:
            display_.showMessage("Buzzer", "Did you hear a tone?", "OK = yes, BACK = no");
            break;
    }
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
        case Status::Waiting:
            return "WAIT";
    }
    return "?";
}
