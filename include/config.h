// Compile-time settings shared across the firmware.

#pragma once

#include <cstdint>

// Must match monitor_speed in platformio.ini
constexpr uint32_t SERIAL_BAUD = 115200;

constexpr uint32_t I2C_CLOCK_HZ = 400000;
