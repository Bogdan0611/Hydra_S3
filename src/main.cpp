// Firmware entry point: boot sequence and main loop.

#include <Arduino.h>

#include "config.h"
#include "core/bus_manager.h"
#include "core/buzzer.h"
#include "core/display_manager.h"
#include "core/input_manager.h"

static DisplayManager display;
static InputManager input;
static Buzzer buzzer;

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

static void showBootScreen() {
    display.clear();
    display.drawCenteredText(28, "Hydra S3");
    display.drawCenteredText(44, "Starting...");
    display.show();
}

// Each component logs its own [OK] / [ERROR] line; this only counts the
// failures. Returns the number of components that failed.
static int bootComponents() {
    int failures = 0;
    if (!BusManager::begin()) {
        failures++;
    }
    if (display.begin()) {
        showBootScreen();
    } else {
        failures++;
    }
    if (!input.begin()) {
        failures++;
    }
    if (!buzzer.begin()) {
        failures++;
    }
    if (!logMemory()) {
        failures++;
    }
    return failures;
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    Serial.println();
    Serial.println("Booting...");

    int failures = bootComponents();
    Serial.printf("[INFO] Boot finished with %d error(s)\n", failures);
}

void loop() {
    InputEvent event = input.poll();
    if (event != InputEvent::None) {
        buzzer.click();
        Serial.printf("[INFO] Input event %d\n", static_cast<int>(event));
    }
    buzzer.update();
}
