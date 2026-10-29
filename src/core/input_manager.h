// Reads the joystick and the SET button and turns presses into events.

#pragma once

#include <cstdint>

/// A single button press. None means no new press since the last poll.
enum class InputEvent { None, Up, Down, Left, Right, Ok, Back };

/// Debounces the six buttons and reports each press once, on the press edge:
/// holding a button down produces a single event.
class InputManager {
public:
    InputManager();

    /// Configures the button pins. Returns false if a button already reads
    /// pressed at boot, which usually means a stuck switch or a short to GND.
    bool begin();

    /// Returns the next press, or InputEvent::None. Call on every loop iteration.
    InputEvent poll();

private:
    struct Button {
        uint8_t pin;
        InputEvent event;
        const char* name;
        bool lastReading = false;  // raw level, true = pressed
        bool pressed = false;      // debounced state
        uint32_t lastChangeMs = 0;
    };

    static constexpr int BUTTON_COUNT = 6;

    Button buttons_[BUTTON_COUNT];

    bool updateButton(Button& button, uint32_t nowMs);
};
