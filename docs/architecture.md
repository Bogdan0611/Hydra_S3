# Firmware architecture

How the firmware is organized and why. New files and apps follow this
structure; if something does not fit, the document is updated deliberately
rather than worked around.

---

## 1. Principles

1. **A single cooperative loop.** `loop()` polls input, runs the active
   app and redraws the screen. Every step is short and non-blocking.
   FreeRTOS tasks are introduced only when a feature genuinely needs one
   (e.g. Wi-Fi callbacks), and are documented here when they are.
2. **One open app, one active radio.** An app releases everything it
   started when it exits. This removes SPI contention and Wi-Fi/BLE radio
   sharing issues by design.
3. **Missing hardware degrades gracefully.** If the SD card or a module
   does not respond, the menu still works; only the app that needs it
   refuses to start, with a clear message.
4. **Correct first, fast later.** Optimize only after a measurement shows
   the need.

---

## 2. Repository layout

```
.
├── platformio.ini
├── README.md, CONTRIBUTING.md, CHANGELOG.md, .clang-format
├── docs/
├── include/
│   ├── pins.h                 # every pin, matching docs/hardware.md
│   └── config.h               # tunables: debounce, default frequency, buffer sizes
└── src/
    ├── main.cpp               # setup() and loop() only
    ├── core/                  # shared infrastructure
    │   ├── bus_manager.*      # I2C and SPI initialization
    │   ├── display_manager.*  # thin wrapper around U8g2
    │   ├── input_manager.*    # joystick edges and debounce → InputEvent
    │   ├── list_view.*        # reusable scrolling list (menu and apps)
    │   ├── storage_manager.*  # SD mount, folders, unique file names
    │   ├── buzzer.*           # short feedback sounds
    │   └── app_manager.*      # main menu; starts and stops apps
    ├── drivers/               # one thin wrapper per chip
    │   ├── cc1101_radio.*
    │   ├── pn532_reader.*
    │   ├── nrf24_radio.*
    │   └── ir_transceiver.*
    └── apps/                  # one folder per menu app
        ├── app.h              # common App interface
        ├── placeholder/       # stand-in for apps of later stages
        ├── hardware_test/
        ├── subghz/
        ├── nfc/
        ├── infrared/
        ├── wifi/
        ├── ble/
        ├── nrf24/
        └── usb_hid/
```

A component's `.h` and `.cpp` live side by side. Only `pins.h` and
`config.h` are in `include/`, since everything uses them.

**Drivers vs apps.** A driver knows *how* to talk to a chip (registers,
SPI, interrupts). An app knows *what* the user wants (screens, files,
buttons). A second app using the same chip reuses the driver instead of
duplicating it.

---

## 3. Boot sequence (`setup()`)

```
1. Serial.begin(115200)              boot log
2. All CS pins → OUTPUT, HIGH        no SPI device selected by accident
3. BusManager: Wire + SPI            before any bus device
4. DisplayManager                    shows a boot screen
5. InputManager
6. Buzzer
7. StorageManager (optional)         a missing card does not stop boot
8. AppManager                        registers apps, shows the menu
```

Each step logs `[OK] <component>` or `[ERROR] <component>: <reason>`.
If a step failed, a notice replaces the menu until the next button press:
a missing SD card is named, any other error points to the Serial log.

---

## 4. The `App` interface

```cpp
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
```

**Screens inside an app** are an `enum class` (e.g. `SubGhzScreen::Menu`,
`SubGhzScreen::RawCapture`) plus a `switch` in `onLoop()` and `onInput()`.
No class hierarchies for screens; an enum and a switch are easier to
follow.

---

## 5. Core components

### 5.1 `InputManager`

- Detects the **press edge** (HIGH → LOW), not the level. Holding a button
  produces a single event.
- **~30 ms debounce per button.** (The old 150 ms value was a repeat-rate
  limit, not a debounce.)
- Returns `enum class InputEvent { None, Up, Down, Left, Right, Ok, Back }`.
- Auto-repeat on held UP/DOWN is a later improvement, not part of v1.

### 5.2 `DisplayManager`

- **U8g2 full-buffer driver:** `U8G2_SSD1306_128X64_NONAME_F_HW_I2C`.
- **Redraws only when state changes.** The menu and each app keep a
  `needsRedraw_` flag and call `show()` only when it is set. A full frame
  is ~1 KB over I2C (~25 ms at 400 kHz); redrawing continuously would
  starve the PN532, which shares the bus.
- **`drawText(x, y, text, inverted)`:** `inverted = true` draws with the
  pixels cleared (`setDrawColor(0)`), then restores the draw color.

### 5.3 `ListView`: one implementation for every list

**Lesson from the old firmware.** The unreadable selected item came from
drawing the highlight box and the text in the same color, and the fix had
to be repeated in every screen with a list. Now a single list component
serves the main menu and every app, so the fix lives in one place.

- Takes an array of labels; UP/DOWN move the selection.
- Sliding window: up to 5 visible rows, with ▲▼ indicators when the list is
  longer.
- Selected row: filled box with inverted text.

### 5.4 `StorageManager`

- **The SD card is optional;** check `isReady()` before any write.
- **Card folders:** `/subghz`, `/nfc`, `/ir`, `/usb_hid`, `/logs`, created
  on mount.
- **Unique names, never overwrite:**
  `nextFreePath("/subghz", "raw", ".sub")` → `/subghz/raw_001.sub`,
  `raw_002.sub`, ...
- **Human-readable text formats.** Where a known Flipper Zero format exists
  it is preferred, for compatibility with community files (details in
  `docs/roadmap.md`).

### 5.5 `Buzzer`

- Short helpers: `click()`, `success()`, `error()`.
- **The active level (HIGH or LOW) is a constant in `config.h`,** set after
  the first-boot check (`docs/hardware.md`, section 6). The pin returns to
  its idle level after every sound.
- Sounds use the LEDC PWM peripheral directly (`ledcWriteTone()`), which
  works with both active and passive buzzers. `tone()` is not used: it
  leaves the pin LOW after every sound (an active-LOW module would keep
  sounding) and ends sounds from a separate FreeRTOS task. For an
  active-LOW module the LEDC output is inverted (`ledcOutputInvert()`), so
  silence (duty 0) is HIGH.
- Sounds never block: `update()`, called from `loop()`, ends each one on
  time.

---

## 6. Shared buses

### 6.1 SPI (CC1101, NRF24, SD)

- All CS pins are HIGH before the first transaction.
- Each driver uses its own `SPISettings` inside
  `beginTransaction()` / `endTransaction()`, so no device inherits another
  one's clock or mode.
- **Safe clocks on jumper wires:** ~4 MHz for CC1101, NRF24 and SD. At
  10 MHz the SD card mounted only intermittently on the bench (FAT volume
  not found). Raise them only after testing proves stability.
- **SmartRC-CC1101 manages SPI itself.** Call `setSpiPin()` before
  `Init()`. Once the CC1101 and the SD card are both integrated, verify
  explicitly that both still respond (init order may matter).

### 6.2 I2C (OLED, PN532)

- 400 kHz. The screen redraws only on changes (5.2).
- The PN532 uses I2C clock stretching heavily. If intermittent errors
  appear, the first fix is a longer timeout (`Wire.setTimeOut()`).

### 6.3 2.4 GHz radio (Wi-Fi, BLE)

- One antenna and one radio. Wi-Fi and BLE apps never run at the same
  time; `onExit()` shuts the respective stack down completely.

---

## 7. Interrupt service routines

Lessons from the old firmware:

- ISRs are `IRAM_ATTR`, so they run from internal RAM even while flash is
  busy.
- **Do the minimum:** read `micros()`, store a value, return. No Serial,
  SD or display access.
- Variables shared with an ISR are `volatile`.
- `attachInterrupt()` takes a free function, not a member function. Use a
  `static` trampoline plus a `static` pointer to the active instance.
- When the buffer is full, the ISR drops new samples and never writes past
  the end.
- **Stop capturing (`detachInterrupt`) before writing to SD,** never the
  other way round.

---

## 8. Memory

- Small and medium buffers live in internal RAM (e.g. 2000 durations ×
  4 bytes = 8 KB).
- PSRAM (8 MB, via `ps_malloc`) is for large buffers only (long captures,
  big scan lists), and only when needed.
- Free internal heap and PSRAM are logged at boot to catch leaks early.

---

## 9. PlatformIO configuration

```ini
; pioarduino fork: official PlatformIO does not support Arduino-ESP32 core 3.x.
; Pinned for reproducible builds; upgrade deliberately, not automatically.
[env:esp32s3]
platform = https://github.com/pioarduino/platform-espressif32/releases/download/55.03.39/platform-espressif32.zip
board = esp32-s3-devkitc-1
framework = arduino

; N16R8: 16 MB QIO flash + 8 MB octal (OPI) PSRAM
board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_upload.flash_size = 16MB
board_build.partitions = default_16MB.csv

monitor_speed = 115200

build_flags =
    -DBOARD_HAS_PSRAM

; Libraries are added only when the stage that needs them starts.
lib_deps =
    olikraus/U8g2@^2.36.18
```

- **Flashing and Serial use the COM port.** Native USB stays free for the
  USB HID stage, so `ARDUINO_USB_CDC_ON_BOOT` is not enabled.
- **If the board does not boot after flashing** (PSRAM error on Serial),
  check `memory_type` first; the board must really be an R8 with octal
  PSRAM.

---

## 10. Libraries

| Module | Library | `lib_deps` | Notes |
|---|---|---|---|
| OLED | U8g2 | `olikraus/U8g2@^2.36.18` | — |
| CC1101 | SmartRC-CC1101-Driver-Lib **3.0.2** | `lsatan/SmartRC-CC1101-Driver-Lib@^3.0.2` | Rewritten in 2026 with an object API (`SmartRC_CC1101 radio;`); 3.0.2 adds core 3.x compatibility. Header: `SmartRC_CC1101.h` |
| PN532 | Adafruit PN532 | `adafruit/Adafruit PN532` | I2C mode, address `0x24` |
| NRF24 | RF24 | `nrf24/RF24` | — |
| IR | decided in the IR stage | — | The latest released IRremoteESP8266 (2.8.6) does not build on core 3.x; main alternative: Arduino-IRremote. Re-check at that point |
| BLE | NimBLE-Arduino | `h2zero/NimBLE-Arduino` | Lighter than the default BLE stack |
| SD | SD (bundled with the core) | — | — |
| USB HID | USB (bundled with the core) | — | `USBHIDKeyboard`, on the native USB port |
