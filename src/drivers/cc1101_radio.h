// CC1101 Sub-GHz transceiver.

#pragma once

#include <cstdint>

/// SPI access to the CC1101. For now it only reads the chip version, to
/// prove the chip answers; the radio functions arrive with the Sub-GHz stage.
class Cc1101Radio {
public:
    /// Reads the VERSION status register into version. Returns false if the
    /// chip never signals ready, which means nothing answers on SPI.
    bool readVersion(uint8_t& version);

private:
    bool waitUntilReady();
};
