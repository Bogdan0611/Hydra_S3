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

/// Logs the address of every device that answers on the I2C bus.
void logI2cDevices();

}  // namespace BusManager
