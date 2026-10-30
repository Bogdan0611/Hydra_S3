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

// After power-on IOCFG0 is 0x3F: GDO0 outputs CLK_XOSC/192, ~135 kHz with
// the module's 26 MHz crystal. That is ~540 edges in 2 ms; a pin without the
// clock shows none.
constexpr uint32_t CLOCK_SAMPLE_US = 2000;
constexpr uint32_t CLOCK_MIN_EDGES = 100;

}  // namespace

bool Cc1101Radio::readVersion(uint8_t& version) {
    SPI.beginTransaction(SPISettings(CC1101_SPI_CLOCK_HZ, MSBFIRST, SPI_MODE0));

    // An SD card keeps driving MISO after its CS goes HIGH, until it receives
    // a clock, and the SD library sends none. The ready check below reads MISO
    // before any clock, so first clock one byte with every chip select HIGH to
    // make the card release the line.
    SPI.transfer(0xFF);

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

bool Cc1101Radio::isPowerOnClockPresent(uint32_t& edgesCounted) {
    // The pull-down keeps a disconnected pin from picking up noise as edges.
    pinMode(PIN_CC1101_GDO0, INPUT_PULLDOWN);
    edgesCounted = 0;

    // Polling samples far faster than the 7.4 us clock period, so no edge is missed.
    int lastLevel = digitalRead(PIN_CC1101_GDO0);
    uint32_t startUs = micros();
    while (micros() - startUs < CLOCK_SAMPLE_US) {
        int level = digitalRead(PIN_CC1101_GDO0);
        if (level != lastLevel) {
            edgesCounted++;
            lastLevel = level;
        }
    }
    return edgesCounted >= CLOCK_MIN_EDGES;
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
