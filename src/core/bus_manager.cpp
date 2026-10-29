// Starts the shared I2C and SPI buses.

#include "core/bus_manager.h"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "config.h"
#include "pins.h"

namespace {

// A floating CS pin can select a module by accident, and two selected
// modules fight over MISO. All of them go HIGH before the first transaction.
void deselectSpiDevices() {
    const uint8_t chipSelectPins[] = {PIN_CC1101_CSN, PIN_NRF24_CSN, PIN_SD_CS};
    for (uint8_t pin : chipSelectPins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
}

bool beginI2c() {
    if (!Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_CLOCK_HZ)) {
        Serial.println("[ERROR] I2C: bus failed to start");
        return false;
    }
    Serial.println("[OK] I2C bus");
    return true;
}

bool beginSpi() {
    // No hardware CS (-1): each driver drives its own chip select.
    if (!SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, -1)) {
        Serial.println("[ERROR] SPI: bus failed to start");
        return false;
    }
    Serial.println("[OK] SPI bus");
    return true;
}

}  // namespace

bool BusManager::begin() {
    deselectSpiDevices();
    bool i2cStarted = beginI2c();
    bool spiStarted = beginSpi();
    return i2cStarted && spiStarted;
}

bool BusManager::isI2cDevicePresent(uint8_t address) {
    // An empty write: a present device acknowledges its address, then the
    // transfer ends. endTransmission() returns 0 only on that acknowledge.
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

void BusManager::logI2cDevices() {
    Serial.print("[INFO] I2C devices found:");
    bool anyFound = false;
    // 0x08-0x77 is the range for normal devices; the rest is reserved.
    for (uint8_t address = 0x08; address <= 0x77; address++) {
        if (isI2cDevicePresent(address)) {
            Serial.printf(" 0x%02X", address);
            anyFound = true;
        }
    }
    if (!anyFound) {
        Serial.print(" none");
    }
    Serial.println();
}
