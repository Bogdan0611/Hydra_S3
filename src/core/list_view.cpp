// Scrolling list with a highlighted selection, shared by the menu and the apps.

#include "core/list_view.h"

namespace {

// Five 12 px rows fill 60 of the 64 px; the 4 spare px are split above and below.
constexpr size_t VISIBLE_ROWS = 5;
constexpr int ROW_HEIGHT = 12;
constexpr int LIST_TOP = 2;
constexpr int LIST_BOTTOM = LIST_TOP + VISIBLE_ROWS * ROW_HEIGHT - 1;

// The 6x10 font has 7 px capitals; this baseline centers them in the row.
constexpr int TEXT_LEFT = 3;
constexpr int TEXT_BASELINE = 9;

// The selection box stops short of the right edge, so an arrow is never drawn
// on top of it in the same color.
constexpr int ROW_WIDTH = 120;
constexpr int ARROW_LEFT_X = 121;
constexpr int ARROW_TIP_X = 124;
constexpr int ARROW_RIGHT_X = 127;
constexpr int ARROW_HEIGHT = 4;

}  // namespace

void ListView::setItems(const char* const* labels, size_t count) {
    labels_ = labels;
    count_ = count;
    selectedIndex_ = 0;
    firstVisibleIndex_ = 0;
}

bool ListView::handleInput(InputEvent event) {
    if (event == InputEvent::Up && selectedIndex_ > 0) {
        selectedIndex_--;
    } else if (event == InputEvent::Down && selectedIndex_ + 1 < count_) {
        selectedIndex_++;
    } else {
        return false;
    }
    scrollToSelection();
    return true;
}

size_t ListView::selectedIndex() const {
    return selectedIndex_;
}

void ListView::draw(DisplayManager& display) const {
    for (size_t row = 0; row < VISIBLE_ROWS; row++) {
        size_t index = firstVisibleIndex_ + row;
        if (index >= count_) {
            break;
        }

        int rowTop = LIST_TOP + static_cast<int>(row) * ROW_HEIGHT;
        bool selected = index == selectedIndex_;
        // Box and text share draw color 1, so selected text must be drawn
        // inverted or it disappears into the box.
        if (selected) {
            display.drawBox(0, rowTop, ROW_WIDTH, ROW_HEIGHT);
        }
        display.drawText(TEXT_LEFT, rowTop + TEXT_BASELINE, labels_[index], selected);
    }
    drawScrollArrows(display);
}

// Moves the window only as far as needed to keep the selected row visible.
void ListView::scrollToSelection() {
    if (selectedIndex_ < firstVisibleIndex_) {
        firstVisibleIndex_ = selectedIndex_;
    } else if (selectedIndex_ >= firstVisibleIndex_ + VISIBLE_ROWS) {
        firstVisibleIndex_ = selectedIndex_ - VISIBLE_ROWS + 1;
    }
}

void ListView::drawScrollArrows(DisplayManager& display) const {
    bool moreAbove = firstVisibleIndex_ > 0;
    bool moreBelow = firstVisibleIndex_ + VISIBLE_ROWS < count_;

    if (moreAbove) {
        int baseY = LIST_TOP + ARROW_HEIGHT;
        display.drawTriangle(ARROW_LEFT_X, baseY, ARROW_RIGHT_X, baseY, ARROW_TIP_X, LIST_TOP);
    }
    if (moreBelow) {
        int baseY = LIST_BOTTOM - ARROW_HEIGHT;
        display.drawTriangle(ARROW_LEFT_X, baseY, ARROW_RIGHT_X, baseY, ARROW_TIP_X, LIST_BOTTOM);
    }
}
