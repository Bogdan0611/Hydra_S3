// Short feedback sounds on the buzzer module.

#include "core/buzzer.h"

#include <Arduino.h>

#include "config.h"
#include "pins.h"

namespace {

// ledcWriteTone() always switches the channel to 10-bit resolution.
constexpr uint8_t PWM_RESOLUTION_BITS = 10;

constexpr uint32_t CLICK_HZ = 3000;
constexpr uint32_t CLICK_MS = 20;
constexpr uint32_t SUCCESS_HZ = 4000;
constexpr uint32_t SUCCESS_MS = 100;
constexpr uint32_t ERROR_HZ = 600;
constexpr uint32_t ERROR_MS = 300;

}  // namespace

bool Buzzer::begin() {
    // tone() is not used: it leaves the pin LOW after every sound, which keeps
    // an active-LOW module sounding, and it stops sounds from another task.
    if (!ledcAttach(PIN_BUZZER, CLICK_HZ, PWM_RESOLUTION_BITS)) {
        Serial.println("[ERROR] Buzzer: no free PWM channel");
        return false;
    }
    // Silence is duty 0, which is a LOW output. For an active-LOW module the
    // output is inverted, so silence becomes HIGH.
    ledcOutputInvert(PIN_BUZZER, !BUZZER_ACTIVE_HIGH);
    ledcWriteTone(PIN_BUZZER, 0);

    ready_ = true;
    Serial.printf("[OK] Buzzer (active %s)\n", BUZZER_ACTIVE_HIGH ? "HIGH" : "LOW");
    return true;
}

void Buzzer::click() {
    play(CLICK_HZ, CLICK_MS);
}

void Buzzer::success() {
    play(SUCCESS_HZ, SUCCESS_MS);
}

void Buzzer::error() {
    play(ERROR_HZ, ERROR_MS);
}

void Buzzer::update() {
    if (!playing_ || millis() - startMs_ < durationMs_) {
        return;
    }
    ledcWriteTone(PIN_BUZZER, 0);
    playing_ = false;
}

void Buzzer::play(uint32_t frequencyHz, uint32_t durationMs) {
    if (!ready_) {
        return;
    }
    ledcWriteTone(PIN_BUZZER, frequencyHz);
    playing_ = true;
    startMs_ = millis();
    durationMs_ = durationMs;
}
