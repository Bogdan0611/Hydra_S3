// Compile-time settings shared across the firmware.

#pragma once

#include <cstdint>

// Must match monitor_speed in platformio.ini
constexpr uint32_t SERIAL_BAUD = 115200;

constexpr uint32_t I2C_CLOCK_HZ = 400000;

// Kept low for jumper wires; raise it only after testing proves it stable
constexpr uint32_t SD_SPI_CLOCK_HZ = 4000000;

// 7-bit address; some SSD1306 modules use 0x3D
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;

// How long a button level must stay unchanged before it counts
constexpr uint32_t BUTTON_DEBOUNCE_MS = 30;

// Level that switches the buzzer module on. Not yet identified: see
// docs/hardware.md, section 6. If an active buzzer sounds continuously
// while idle, this value is wrong.
constexpr bool BUZZER_ACTIVE_HIGH = true;
