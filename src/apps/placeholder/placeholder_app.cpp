// Menu entry for an app that is not implemented yet.

#include "apps/placeholder/placeholder_app.h"

PlaceholderApp::PlaceholderApp(DisplayManager& display, const char* name)
    : display_(display), name_(name) {}

const char* PlaceholderApp::name() const {
    return name_;
}

bool PlaceholderApp::onEnter() {
    needsRedraw_ = true;
    return true;
}

// Nothing to release: the placeholder uses no hardware.
void PlaceholderApp::onExit() {}

void PlaceholderApp::onLoop() {
    if (!needsRedraw_) {
        return;
    }
    display_.showMessage(name_, "Coming soon", "BACK to return");
    needsRedraw_ = false;
}

bool PlaceholderApp::onInput(InputEvent event) {
    return event != InputEvent::Back;
}
