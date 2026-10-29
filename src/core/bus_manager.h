// Starts the shared I2C and SPI buses.

#pragma once

#include <cstdint>

namespace BusManager {

/// Deselects every SPI device, then starts the I2C and SPI buses on the pins
/// from pins.h. Must run before any device on either bus is initialized.
/// Returns false if a bus fails to start.
bool begin();

/// True if a device acknowledges the given 7-bit I2C address.
bool isI2cDevicePresent(uint8_t address);

}  // namespace BusManager
