// Infrared receiver and LED.

#pragma once

#include <cstdint>

/// For now it only counts edges on the receiver output, to prove the receiver
/// reacts to a remote; decoding and sending arrive with the infrared stage.
class IrTransceiver {
public:
    /// Configures the receiver pin and starts counting its edges from zero.
    void startEdgeCount();

    /// Stops counting. Safe to call when counting was never started.
    void stopEdgeCount();

    /// Edges seen since startEdgeCount().
    uint32_t edgeCount() const;

    /// True if the receiver output is at its idle level (HIGH).
    bool isIdle() const;

private:
    bool counting_ = false;
};
