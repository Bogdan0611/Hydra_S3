// CC1101 Sub-GHz transceiver.

#pragma once

#include <cstdint>

/// Access to the CC1101. For now it only checks that the chip runs and
/// answers on SPI; the radio functions arrive with the Sub-GHz stage.
class Cc1101Radio {
public:
    /// Reads the VERSION status register into version. Returns false if the
    /// chip never signals ready, which means nothing answers on SPI.
    bool readVersion(uint8_t& version);

    /// True if GDO0 carries the ~135 kHz clock the CC1101 outputs after power-on,
    /// which proves the chip is powered and running without using SPI at all.
    /// Only meaningful while the chip is unconfigured (after a power cycle).
    /// edgesCounted receives the number of edges seen, for the log.
    bool isPowerOnClockPresent(uint32_t& edgesCounted);

private:
    bool waitUntilReady();
};
