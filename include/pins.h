// Every GPIO used by the firmware. Must match docs/hardware.md, section 3;
// no other file uses raw GPIO numbers.

#pragma once

#include <cstdint>

// Joystick and SET button: INPUT_PULLUP, pressed = LOW
constexpr uint8_t PIN_BUTTON_UP = 1;
constexpr uint8_t PIN_BUTTON_DOWN = 2;
constexpr uint8_t PIN_BUTTON_LEFT = 5;
constexpr uint8_t PIN_BUTTON_RIGHT = 6;
constexpr uint8_t PIN_BUTTON_OK = 7;
constexpr uint8_t PIN_BUTTON_BACK = 17;  // labelled SET on the board

// I2C bus: OLED + PN532
constexpr uint8_t PIN_I2C_SDA = 8;
constexpr uint8_t PIN_I2C_SCL = 9;

// SPI bus: CC1101 + NRF24 + MicroSD (default FSPI pins, routed through the IO MUX)
constexpr uint8_t PIN_SPI_MOSI = 11;
constexpr uint8_t PIN_SPI_SCK = 12;
constexpr uint8_t PIN_SPI_MISO = 13;

// CC1101 (Sub-GHz)
constexpr uint8_t PIN_CC1101_CSN = 10;
constexpr uint8_t PIN_CC1101_GDO0 = 4;
constexpr uint8_t PIN_CC1101_GDO2 = 18;

// NRF24L01+ (2.4 GHz)
constexpr uint8_t PIN_NRF24_CSN = 14;
constexpr uint8_t PIN_NRF24_CE = 15;
constexpr uint8_t PIN_NRF24_IRQ = 16;

// MicroSD
constexpr uint8_t PIN_SD_CS = 21;

// PN532 (NFC); may share the pin with the on-board RGB LED
constexpr uint8_t PIN_PN532_IRQ = 38;

// Infrared
constexpr uint8_t PIN_IR_RX = 40;
constexpr uint8_t PIN_IR_TX = 41;

// Buzzer
constexpr uint8_t PIN_BUZZER = 42;
