// Reads the joystick and the SET button and turns presses into events.

#include "core/input_manager.h"

#include <Arduino.h>

#include "config.h"
#include "pins.h"

InputManager::InputManager()
    : buttons_{
          {PIN_BUTTON_UP, InputEvent::Up, "UP"},
          {PIN_BUTTON_DOWN, InputEvent::Down, "DOWN"},
          {PIN_BUTTON_LEFT, InputEvent::Left, "LEFT"},
          {PIN_BUTTON_RIGHT, InputEvent::Right, "RIGHT"},
          {PIN_BUTTON_OK, InputEvent::Ok, "OK"},
          {PIN_BUTTON_BACK, InputEvent::Back, "BACK"},
      } {}

bool InputManager::begin() {
    for (Button& button : buttons_) {
        pinMode(button.pin, INPUT_PULLUP);
    }

    bool allReleased = true;
    for (Button& button : buttons_) {
        // Starting from the real level means a button held at boot fires
        // nothing until it is released and pressed again.
        button.lastReading = digitalRead(button.pin) == LOW;
        button.pressed = button.lastReading;
        if (button.pressed) {
            Serial.printf("[ERROR] Input: %s pressed at boot (stuck switch or short to GND)\n",
                          button.name);
            allReleased = false;
        }
    }

    if (allReleased) {
        Serial.println("[OK] Input");
    }
    return allReleased;
}

InputEvent InputManager::poll() {
    uint32_t nowMs = millis();
    for (Button& button : buttons_) {
        if (updateButton(button, nowMs)) {
            return button.event;
        }
    }
    return InputEvent::None;
}

// Returns true only on a debounced press edge (released -> pressed).
bool InputManager::updateButton(Button& button, uint32_t nowMs) {
    bool reading = digitalRead(button.pin) == LOW;
    if (reading != button.lastReading) {
        button.lastReading = reading;
        button.lastChangeMs = nowMs;
        return false;
    }

    // Contacts bounce for a few ms; a level counts only once it has been stable.
    bool stable = nowMs - button.lastChangeMs >= BUTTON_DEBOUNCE_MS;
    if (!stable || reading == button.pressed) {
        return false;
    }
    button.pressed = reading;
    return button.pressed;
}
