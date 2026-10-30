// NRF24L01+ 2.4 GHz transceiver.

#include "drivers/nrf24_radio.h"

#include <Arduino.h>
#include <SPI.h>

#include "config.h"
#include "pins.h"

namespace {

constexpr uint8_t R_REGISTER = 0x00;   // command; the low 5 bits carry the address
constexpr uint8_t NOP_COMMAND = 0xFF;  // clocks out the register value
constexpr uint8_t SETUP_AW_REGISTER = 0x03;

}  // namespace

void Nrf24Radio::begin() {
    pinMode(PIN_NRF24_CE, OUTPUT);
    digitalWrite(PIN_NRF24_CE, LOW);
}

bool Nrf24Radio::isChipConnected(uint8_t& setupAw) {
    // SETUP_AW selects 3, 4 or 5 byte addresses and only accepts 1-3. Without a
    // chip MISO reads 0x00 or 0xFF, both outside that range.
    setupAw = readRegister(SETUP_AW_REGISTER);
    return setupAw >= 1 && setupAw <= 3;
}

uint8_t Nrf24Radio::readRegister(uint8_t address) {
    SPI.beginTransaction(SPISettings(NRF24_SPI_CLOCK_HZ, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_NRF24_CSN, LOW);
    SPI.transfer(R_REGISTER | address);  // the chip answers this byte with STATUS
    uint8_t value = SPI.transfer(NOP_COMMAND);
    digitalWrite(PIN_NRF24_CSN, HIGH);
    SPI.endTransaction();
    return value;
}
