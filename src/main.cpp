// Firmware entry point: boot sequence and main loop.

#include <Arduino.h>

#include "apps/placeholder/placeholder_app.h"
#include "config.h"
#include "core/app_manager.h"
#include "core/bus_manager.h"
#include "core/buzzer.h"
#include "core/display_manager.h"
#include "core/input_manager.h"
#include "core/storage_manager.h"

static DisplayManager display;
static InputManager input;
static Buzzer buzzer;
static StorageManager storage;
static AppManager appManager(display);

// Stand-ins for the apps of later stages; each is replaced when its stage starts.
static PlaceholderApp hardwareTestApp(display, "Hardware Test");
static PlaceholderApp subGhzApp(display, "Sub-GHz");
static PlaceholderApp nfcApp(display, "NFC");
static PlaceholderApp infraredApp(display, "Infrared");
static PlaceholderApp bleApp(display, "BLE");
static PlaceholderApp wifiApp(display, "Wi-Fi");
static PlaceholderApp nrf24App(display, "NRF24");
static PlaceholderApp usbHidApp(display, "USB HID");

// Returns false if the PSRAM was not detected.
static bool logMemory() {
    Serial.printf("[INFO] Free heap: %lu KB internal\n", ESP.getFreeHeap() / 1024);

    if (!psramFound()) {
        Serial.println("[ERROR] PSRAM: not found (check memory_type in platformio.ini)");
        return false;
    }
    Serial.printf("[OK] PSRAM: %lu KB free of %lu KB\n", ESP.getFreePsram() / 1024,
                  ESP.getPsramSize() / 1024);
    return true;
}

// Each component logs its own [OK] / [ERROR] line; this only counts the
// failures. Returns the number of components that failed.
static int bootComponents() {
    int failures = 0;
    if (!BusManager::begin()) {
        failures++;
    }
    if (display.begin()) {
        display.showMessage("Hydra S3", "Starting...", "");
    } else {
        failures++;
    }
    if (!input.begin()) {
        failures++;
    }
    if (!buzzer.begin()) {
        failures++;
    }
    // A missing SD card is logged and counted, but the firmware keeps running.
    if (!storage.begin()) {
        failures++;
    }
    if (!logMemory()) {
        failures++;
    }
    return failures;
}

static void registerApps() {
    appManager.addApp(hardwareTestApp);
    appManager.addApp(subGhzApp);
    appManager.addApp(nfcApp);
    appManager.addApp(infraredApp);
    appManager.addApp(bleApp);
    appManager.addApp(wifiApp);
    appManager.addApp(nrf24App);
    appManager.addApp(usbHidApp);
    appManager.begin();
}

// The details of a boot error are only on Serial. The screen names the one
// a user can fix on the spot (the SD card), or points to the log otherwise.
static void showBootNotice(int failures) {
    if (!storage.isReady()) {
        appManager.showNotice("SD card unavailable", "Saving is disabled");
    } else if (failures > 0) {
        appManager.showNotice("Boot errors found", "See the Serial log");
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    Serial.println();
    Serial.println("Booting...");

    int failures = bootComponents();
    registerApps();
    showBootNotice(failures);
    Serial.printf("[INFO] Boot finished with %d error(s)\n", failures);
}

void loop() {
    InputEvent event = input.poll();
    if (event != InputEvent::None) {
        buzzer.click();
        appManager.handleInput(event);
    }
    appManager.update();
    buzzer.update();
}
