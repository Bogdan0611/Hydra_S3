// Scrolling list with a highlighted selection, shared by the menu and the apps.

#pragma once

#include <cstddef>

#include "core/display_manager.h"
#include "core/input_manager.h"

/// Shows up to five rows of a list of labels. UP and DOWN move the selection
/// and the visible window follows it; arrows on the right show that more rows
/// exist above or below.
class ListView {
public:
    /// Sets the labels and selects the first one. The array is not copied, so
    /// it must stay valid while the list is in use.
    void setItems(const char* const* labels, size_t count);

    /// Handles UP and DOWN; other events are ignored. Returns true if the
    /// selection changed, i.e. the list needs to be redrawn.
    bool handleInput(InputEvent event);

    /// Index of the selected label.
    size_t selectedIndex() const;

    /// Draws the visible rows into the frame buffer. Does not call show().
    void draw(DisplayManager& display) const;

private:
    const char* const* labels_ = nullptr;
    size_t count_ = 0;
    size_t selectedIndex_ = 0;
    size_t firstVisibleIndex_ = 0;

    void scrollToSelection();
    void drawScrollArrows(DisplayManager& display) const;
};
