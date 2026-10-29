// MicroSD card: mounting and the folder layout.

#include "core/storage_manager.h"

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include "config.h"
#include "pins.h"

namespace {

const char* const APP_FOLDERS[] = {"/subghz", "/nfc", "/ir", "/usb_hid", "/logs"};

const char* cardTypeName(sdcard_type_t type) {
    switch (type) {
        case CARD_MMC:
            return "MMC";
        case CARD_SD:
            return "SDSC";
        case CARD_SDHC:
            return "SDHC/SDXC";
        default:
            return "unknown";
    }
}

}  // namespace

bool StorageManager::begin() {
    if (!SD.begin(PIN_SD_CS, SPI, SD_SPI_CLOCK_HZ)) {
        Serial.println("[ERROR] SD card: mount failed (no card, not FAT32, or wiring)");
        return false;
    }
    logCardInfo();

    ready_ = createFolders();
    return ready_;
}

bool StorageManager::isReady() const {
    return ready_;
}

void StorageManager::logCardInfo() {
    uint64_t sizeMb = SD.cardSize() / (1024 * 1024);
    Serial.printf("[OK] SD card: %s, %llu MB\n", cardTypeName(SD.cardType()), sizeMb);
}

bool StorageManager::createFolders() {
    bool allCreated = true;
    for (const char* folder : APP_FOLDERS) {
        // mkdir() fails on a folder that already exists, so check first.
        if (!SD.exists(folder) && !SD.mkdir(folder)) {
            Serial.printf("[ERROR] SD card: cannot create %s\n", folder);
            allCreated = false;
        }
    }
    return allCreated;
}
