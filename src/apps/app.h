// The interface every menu app implements.

#pragma once

#include "core/input_manager.h"

/// Common interface for every app in the main menu.
/// The menu only starts, stops and forwards input; it knows nothing
/// about what an app does internally.
class App {
public:
    virtual ~App() = default;

    /// Name shown in the main menu.
    virtual const char* name() const = 0;

    /// Called once when the app opens. Acquires the hardware it needs.
    /// Returns false if that hardware is unavailable; the app stays closed.
    virtual bool onEnter() = 0;

    /// Called once when the app closes. Must release everything started
    /// in onEnter() (radios, interrupts, Wi-Fi).
    virtual void onExit() = 0;

    /// Called on every loop iteration while the app is open. Must not block.
    virtual void onLoop() = 0;

    /// Called on every button press. Returns false to request closing
    /// (typically BACK on the app's top-level screen).
    virtual bool onInput(InputEvent event) = 0;
};
