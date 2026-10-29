// NRF24L01+ 2.4 GHz transceiver.

#pragma once

#include <cstdint>

/// SPI access to the NRF24L01+. For now it only checks that the chip answers;
/// the radio functions arrive with the NRF24 stage.
class Nrf24Radio {
public:
    /// Holds CE LOW, so the radio can neither transmit nor receive.
    void begin();

    /// Reads the SETUP_AW register into setupAw. Returns true if it holds a
    /// valid value (1-3), i.e. a chip is answering on SPI.
    bool isChipConnected(uint8_t& setupAw);

private:
    uint8_t readRegister(uint8_t address);
};
