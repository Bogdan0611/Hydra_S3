// Hardware Test app: checks every module and shows PASS or FAIL.

#pragma once

#include <cstddef>
#include <cstdint>

#include "apps/app.h"
#include "core/display_manager.h"
#include "core/list_view.h"
#include "core/storage_manager.h"
#include "drivers/cc1101_radio.h"
#include "drivers/nrf24_radio.h"

/// Checks every module when opened and lists the results. OK on a row shows
/// its details and, for a failed check, a hint at the likely cause. Every
/// result is also logged on Serial.
class HardwareTestApp : public App {
public:
    HardwareTestApp(DisplayManager& display, StorageManager& storage);

    const char* name() const override;
    bool onEnter() override;
    void onExit() override;
    void onLoop() override;
    bool onInput(InputEvent event) override;

private:
    enum class Screen { Results, Details };
    enum class Status { Pass, Fail };

    // Rows in display order; also the index into results_.
    enum CheckId : size_t { Oled, Pn532, SdCard, Cc1101, Nrf24, CheckCount };

    // 21 characters fit across the screen, plus the terminating '\0'
    static constexpr size_t LINE_LENGTH = 22;

    struct CheckResult {
        const char* name = "";
        Status status = Status::Fail;
        char detail[LINE_LENGTH] = "";
        const char* hint = "";
        char label[LINE_LENGTH] = "";  // list row: name and status
    };

    DisplayManager& display_;
    StorageManager& storage_;
    Cc1101Radio cc1101_;
    Nrf24Radio nrf24_;

    CheckResult results_[CheckCount];
    const char* labels_[CheckCount] = {};
    ListView list_;
    Screen screen_ = Screen::Results;
    bool needsRedraw_ = false;

    void runChecks();
    void checkI2cDevice(CheckId id, uint8_t address, const char* hint);
    void checkSdCard();
    void checkCc1101();
    void checkNrf24();
    void setResult(CheckId id, Status status, const char* detail, const char* hint);
    bool handleResultsInput(InputEvent event);
    void draw();
    void drawDetails();

    static const char* statusText(Status status);
};
