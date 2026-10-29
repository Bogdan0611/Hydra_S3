// MicroSD card: mounting and the folder layout.

#include "core/storage_manager.h"

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include "config.h"
#include "pins.h"

namespace {

const char* const APP_FOLDERS[] = {"/subghz", "/nfc", "/ir", "/usb_hid", "/logs"};

const char* const SELF_TEST_PATH = "/logs/selftest.tmp";
constexpr size_t SELF_TEST_SIZE = 512;  // one SD sector

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
    // SD.begin() returns true at once if a card was mounted before, even if it
    // has been removed since. Unmounting first makes every call a real check.
    SD.end();
    ready_ = false;

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

bool StorageManager::testReadWrite() {
    if (!ready_) {
        return false;
    }

    // Every byte value appears twice, so a stuck or flipped data bit shows up.
    uint8_t written[SELF_TEST_SIZE];
    for (size_t i = 0; i < SELF_TEST_SIZE; i++) {
        written[i] = static_cast<uint8_t>(i);
    }
    uint8_t readBack[SELF_TEST_SIZE] = {};

    File file = SD.open(SELF_TEST_PATH, FILE_WRITE);
    bool writeDone = file && file.write(written, SELF_TEST_SIZE) == SELF_TEST_SIZE;
    file.close();

    file = SD.open(SELF_TEST_PATH, FILE_READ);
    bool readDone = file && file.read(readBack, SELF_TEST_SIZE) == SELF_TEST_SIZE;
    file.close();
    SD.remove(SELF_TEST_PATH);

    bool matches = writeDone && readDone && memcmp(written, readBack, SELF_TEST_SIZE) == 0;
    if (!matches) {
        Serial.println("[ERROR] SD card: read/write self-test failed");
    }
    return matches;
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
