// Infrared receiver and LED.

#include "drivers/ir_transceiver.h"

#include <Arduino.h>

#include "pins.h"

namespace {

// Written by the interrupt, read by the main loop. A 32-bit read is atomic on
// the ESP32-S3, so no lock is needed.
volatile uint32_t edgesSeen = 0;

void IRAM_ATTR onReceiverEdge() {
    edgesSeen = edgesSeen + 1;  // ++ on a volatile is deprecated since C++20
}

}  // namespace

void IrTransceiver::startEdgeCount() {
    // The pull-up keeps a disconnected pin HIGH, so it never fakes activity.
    pinMode(PIN_IR_RX, INPUT_PULLUP);
    edgesSeen = 0;
    attachInterrupt(PIN_IR_RX, onReceiverEdge, CHANGE);
    counting_ = true;
}

void IrTransceiver::stopEdgeCount() {
    if (!counting_) {
        return;
    }
    detachInterrupt(PIN_IR_RX);
    counting_ = false;
}

uint32_t IrTransceiver::edgeCount() const {
    return edgesSeen;
}

bool IrTransceiver::isIdle() const {
    return digitalRead(PIN_IR_RX) == HIGH;
}
