// Short feedback sounds on the buzzer module.

#pragma once

#include <cstdint>

/// Plays short tones without blocking. update() must run on every loop
/// iteration so each tone ends on time and the pin returns to its idle level.
class Buzzer {
public:
    /// Attaches the buzzer pin to a PWM channel, silent.
    /// Returns false if no PWM channel is free.
    bool begin();

    /// Very short tick, for a key press.
    void click();

    /// High tone, for a finished action.
    void success();

    /// Low, longer tone, for a failed action.
    void error();

    /// Silences the current tone once its duration has passed.
    void update();

private:
    bool ready_ = false;
    bool playing_ = false;
    uint32_t startMs_ = 0;
    uint32_t durationMs_ = 0;

    void play(uint32_t frequencyHz, uint32_t durationMs);
};
