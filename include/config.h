// Compile-time settings shared across the firmware.

#pragma once

#include <cstdint>

// Must match monitor_speed in platformio.ini
constexpr uint32_t SERIAL_BAUD = 115200;

constexpr uint32_t I2C_CLOCK_HZ = 400000;

// 7-bit address; some SSD1306 modules use 0x3D
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;
