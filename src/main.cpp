// Firmware entry point: boot sequence and main loop.

#include <Arduino.h>

#include "config.h"

static void logMemory() {
    Serial.printf("[INFO] Free heap: %lu KB internal\n", ESP.getFreeHeap() / 1024);

    if (psramFound()) {
        Serial.printf("[OK] PSRAM: %lu KB free of %lu KB\n", ESP.getFreePsram() / 1024,
                      ESP.getPsramSize() / 1024);
    } else {
        Serial.println("[ERROR] PSRAM: not found (check memory_type in platformio.ini)");
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    Serial.println();
    Serial.println("Booting...");

    logMemory();
}

void loop() {
}
