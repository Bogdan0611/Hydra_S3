// Main menu: lists the apps, opens the selected one and returns from it.

#pragma once

#include <cstddef>

#include "apps/app.h"
#include "core/display_manager.h"
#include "core/input_manager.h"
#include "core/list_view.h"

/// Owns the main menu. While an app is open, every press and every loop
/// iteration go to that app; when it asks to close, the menu comes back.
class AppManager {
public:
    explicit AppManager(DisplayManager& display);

    /// Adds an app to the menu, in order. The app is not copied, so it must
    /// outlive the manager. Logs an error and skips the app if the menu is full.
    void addApp(App& app);

    /// Shows the menu. Call once, after every app has been added.
    void begin();

    /// Sends a press to the open app, or to the menu if no app is open.
    void handleInput(InputEvent event);

    /// Runs the open app, or redraws the menu if it changed. Call on every
    /// loop iteration.
    void update();

private:
    static constexpr size_t MAX_APPS = 12;

    DisplayManager& display_;
    ListView menu_;
    App* apps_[MAX_APPS] = {};
    const char* appNames_[MAX_APPS] = {};
    size_t appCount_ = 0;
    App* openApp_ = nullptr;
    bool needsRedraw_ = false;

    void openSelectedApp();
    void closeOpenApp();
};
