// Main menu: lists the apps, opens the selected one and returns from it.

#include "core/app_manager.h"

#include <Arduino.h>

AppManager::AppManager(DisplayManager& display) : display_(display) {}

void AppManager::addApp(App& app) {
    if (appCount_ >= MAX_APPS) {
        Serial.printf("[ERROR] Menu: no room for %s (MAX_APPS is %zu)\n", app.name(), MAX_APPS);
        return;
    }
    apps_[appCount_] = &app;
    appNames_[appCount_] = app.name();
    appCount_++;
}

void AppManager::begin() {
    menu_.setItems(appNames_, appCount_);
    needsRedraw_ = true;
    Serial.printf("[OK] Menu: %zu apps\n", appCount_);
}

void AppManager::showNotice(const char* title, const char* detail) {
    noticeTitle_ = title;
    noticeDetail_ = detail;
    needsRedraw_ = true;
}

void AppManager::handleInput(InputEvent event) {
    if (openApp_ != nullptr) {
        bool keepOpen = openApp_->onInput(event);
        if (!keepOpen) {
            closeOpenApp();
        }
        return;
    }

    // The press that dismisses a notice must not also move the menu.
    if (noticeTitle_ != nullptr) {
        noticeTitle_ = nullptr;
        needsRedraw_ = true;
        return;
    }

    if (event == InputEvent::Ok) {
        openSelectedApp();
    } else if (menu_.handleInput(event)) {
        needsRedraw_ = true;
    }
}

void AppManager::update() {
    if (openApp_ != nullptr) {
        openApp_->onLoop();
        return;
    }
    if (!needsRedraw_) {
        return;
    }
    if (noticeTitle_ != nullptr) {
        display_.showMessage(noticeTitle_, noticeDetail_, "Press any button");
    } else {
        drawMenu();
    }
    needsRedraw_ = false;
}

void AppManager::openSelectedApp() {
    if (appCount_ == 0) {
        return;
    }
    App* app = apps_[menu_.selectedIndex()];
    // The app logs the specific reason; the menu simply stays on screen.
    if (!app->onEnter()) {
        Serial.printf("[ERROR] Menu: %s failed to open\n", app->name());
        return;
    }
    Serial.printf("[INFO] Menu: opened %s\n", app->name());
    openApp_ = app;
}

void AppManager::closeOpenApp() {
    openApp_->onExit();
    Serial.printf("[INFO] Menu: closed %s\n", openApp_->name());
    openApp_ = nullptr;
    needsRedraw_ = true;
}

void AppManager::drawMenu() {
    display_.clear();
    menu_.draw(display_);
    display_.show();
}
