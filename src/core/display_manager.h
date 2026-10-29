// Thin wrapper around U8g2 for the 128x64 SSD1306 OLED.

#pragma once

#include <U8g2lib.h>

/// Draws into a frame buffer in RAM. Nothing appears on the OLED until show()
/// sends the buffer, so a screen is drawn completely before it is shown.
class DisplayManager {
public:
    static constexpr int WIDTH = 128;
    static constexpr int HEIGHT = 64;

    /// Checks that the OLED answers on I2C, then initializes it.
    /// Requires the I2C bus to be started. Returns false if the OLED is missing.
    bool begin();

    /// Clears the frame buffer.
    void clear();

    /// Draws text with its baseline at y. With inverted = true the text pixels
    /// are cleared instead of set, so the text stays readable on a filled box.
    void drawText(int x, int y, const char* text, bool inverted = false);

    /// Draws text horizontally centered, with its baseline at y.
    void drawCenteredText(int y, const char* text);

    /// Draws a filled rectangle.
    void drawBox(int x, int y, int width, int height);

    /// Draws a filled triangle.
    void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2);

    /// Sends the frame buffer to the OLED. A full frame is ~1 KB over I2C,
    /// so callers only call this when the screen content has changed.
    void show();

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2_{U8G2_R0};
    bool ready_ = false;
};
