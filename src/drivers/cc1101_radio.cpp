// CC1101 Sub-GHz transceiver.

#include "drivers/cc1101_radio.h"

#include <Arduino.h>
#include <SPI.h>
#include <driver/gpio.h>

#include "config.h"
#include "pins.h"

namespace {

constexpr uint8_t VERSION_REGISTER = 0x31;

// Addresses 0x30-0x3D are command strobes unless the burst bit is set; with
// it they read the status registers instead (datasheet, SPI address space).
constexpr uint8_t READ_BURST = 0xC0;

constexpr uint32_t READY_TIMEOUT_US = 1000;

}  // namespace

bool Cc1101Radio::readVersion(uint8_t& version) {
    SPI.beginTransaction(SPISettings(CC1101_SPI_CLOCK_HZ, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_CC1101_CSN, LOW);

    bool ready = waitUntilReady();
    if (ready) {
        SPI.transfer(VERSION_REGISTER | READ_BURST);
        version = SPI.transfer(0x00);
    }

    digitalWrite(PIN_CC1101_CSN, HIGH);
    SPI.endTransaction();
    return ready;
}

// After CSN goes LOW, the CC1101 holds MISO HIGH until its crystal oscillator
// is running; only then may the first byte be sent. Without a chip MISO never
// goes LOW, hence the timeout. digitalRead() would log a warning on a pin
// owned by SPI, so the pad level is read directly.
bool Cc1101Radio::waitUntilReady() {
    uint32_t startUs = micros();
    while (gpio_get_level(static_cast<gpio_num_t>(PIN_SPI_MISO)) == 1) {
        if (micros() - startUs > READY_TIMEOUT_US) {
            return false;
        }
    }
    return true;
}
