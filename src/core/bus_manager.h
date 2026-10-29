// Starts the shared I2C and SPI buses.

#pragma once

namespace BusManager {

/// Deselects every SPI device, then starts the I2C and SPI buses on the pins
/// from pins.h. Must run before any device on either bus is initialized.
/// Returns false if a bus fails to start.
bool begin();

}  // namespace BusManager
