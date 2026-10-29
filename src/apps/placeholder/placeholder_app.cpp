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
    draw();
    needsRedraw_ = false;
}

bool PlaceholderApp::onInput(InputEvent event) {
    return event != InputEvent::Back;
}

void PlaceholderApp::draw() {
    display_.clear();
    display_.drawCenteredText(20, name_);
    display_.drawCenteredText(36, "Coming soon");
    display_.drawCenteredText(58, "BACK to return");
    display_.show();
}
