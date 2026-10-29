// Thin wrapper around U8g2 for the 128x64 SSD1306 OLED.

#include "core/display_manager.h"

#include <Arduino.h>
#include <Wire.h>

#include "config.h"

bool DisplayManager::begin() {
    // u8g2.begin() succeeds even with no OLED connected, so probe the address first.
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    if (Wire.endTransmission() != 0) {
        Serial.printf("[ERROR] Display: no answer at 0x%02X (check wiring, or try 0x3D)\n",
                      OLED_I2C_ADDRESS);
        return false;
    }

    u8g2_.setI2CAddress(OLED_I2C_ADDRESS << 1);  // U8g2 expects the 8-bit form
    // U8g2 re-applies its own clock before every transfer. Keeping it equal to
    // the bus clock stops it from changing the speed under the PN532.
    u8g2_.setBusClock(I2C_CLOCK_HZ);

    if (!u8g2_.begin()) {
        Serial.println("[ERROR] Display: initialization failed");
        return false;
    }
    u8g2_.setFont(u8g2_font_6x10_tr);
    u8g2_.setFontMode(1);  // transparent: only the glyph pixels are drawn

    ready_ = true;
    Serial.println("[OK] Display");
    return true;
}

void DisplayManager::clear() {
    u8g2_.clearBuffer();
}

void DisplayManager::drawText(int x, int y, const char* text, bool inverted) {
    if (inverted) {
        u8g2_.setDrawColor(0);
    }
    u8g2_.drawStr(x, y, text);
    u8g2_.setDrawColor(1);
}

void DisplayManager::drawCenteredText(int y, const char* text) {
    int x = (WIDTH - u8g2_.getStrWidth(text)) / 2;
    drawText(x, y, text);
}

void DisplayManager::drawBox(int x, int y, int width, int height) {
    u8g2_.drawBox(x, y, width, height);
}

void DisplayManager::drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2) {
    u8g2_.drawTriangle(x0, y0, x1, y1, x2, y2);
}

void DisplayManager::show() {
    // Without an OLED every transfer would only produce I2C errors.
    if (!ready_) {
        return;
    }
    u8g2_.sendBuffer();
}
