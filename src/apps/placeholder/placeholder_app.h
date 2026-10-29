// Menu entry for an app that is not implemented yet.

#pragma once

#include "apps/app.h"
#include "core/display_manager.h"

/// Stands in for an app from a later roadmap stage. Opening it shows a
/// "Coming soon" screen; BACK returns to the menu.
class PlaceholderApp : public App {
public:
    /// The name is not copied, so it must outlive the app (a string literal).
    PlaceholderApp(DisplayManager& display, const char* name);

    const char* name() const override;
    bool onEnter() override;
    void onExit() override;
    void onLoop() override;
    bool onInput(InputEvent event) override;

private:
    DisplayManager& display_;
    const char* name_;
    bool needsRedraw_ = false;

    void draw();
};
