# Roadmap

Stages are listed in implementation order. **A stage is done only when
every "Done when" item has been verified on the hardware.** Each stage is
developed on its own branch, merged through a pull request and tagged
(see `CONTRIBUTING.md`).

All features are tested only on equipment the author owns: his own
remotes, cards, networks and computers.

| Stage | Branch | Tag |
|---|---|---|
| 0 — Foundation | `stage/0-foundation` | `v0.1.0` |
| 1 — Hardware Test | `stage/1-hardware-test` | `v0.2.0` |
| 2 — Sub-GHz | `stage/2-subghz` | `v0.3.0` |
| 3 — NFC | `stage/3-nfc` | `v0.4.0` |
| 4 — Infrared | `stage/4-infrared` | `v0.5.0` |
| 5 — BLE | `stage/5-ble` | `v0.6.0` |
| 6 — Wi-Fi | `stage/6-wifi` | `v0.7.0` |
| 7 — NRF24 | `stage/7-nrf24` | `v0.8.0` |
| 8 — USB HID | `stage/8-usb-hid` | `v0.9.0` |
| 9 — Extensions | per feature | `v0.10.0`+ |

---

## [ ] Stage 0 — Foundation

**Scope:**
- **Repository:** `platformio.ini`, `.gitignore`, `.clang-format`,
  `README.md`, `CONTRIBUTING.md`, `CHANGELOG.md`.
- **CI:** a GitHub Actions workflow that runs `pio run` on every push and
  pull request, with a build-status badge in the README.
- **Firmware:** `include/pins.h`, `include/config.h`, everything in
  `src/core/` (see `docs/architecture.md`), and an empty placeholder app in
  the menu.

**Done when:**
- [ ] The CI build is green on `main`.
- [x] Boot prints `[OK]` / `[ERROR]` for every component on Serial.
- [ ] The menu is navigable with the joystick; the selected row is readable
      (inverted text).
- [ ] The list scrolls correctly with more than 5 (test) entries.
- [ ] Short beep on key press, using the identified buzzer active level.
- [ ] The SD card is detected and its folders are created; without a card
      the menu still works.
- [x] Free internal heap and PSRAM are printed at boot.

---

## [ ] Stage 1 — Hardware Test app

The first real app: checks every module and shows PASS or FAIL. Useful for
debugging for the life of the project, and a good demo.

**Scope:** I2C scan, SD mount, CC1101 VERSION register, NRF24
`isChipConnected()`, IR RX activity, buzzer test. Expected results are in
`docs/hardware.md`, section 6.

**Done when:**
- [ ] Every module shows PASS on screen and on Serial.
- [ ] A deliberately disconnected module shows FAIL with a hint at the
      cause (e.g. "PN532 not responding: check I2C mode switches").

---

## [ ] Stage 2 — Sub-GHz (CC1101)

### 2.1 Raw capture

Listen on a frequency and record the durations of the demodulated signal.

**Configuration used by the old firmware** (re-check against the
SmartRC 3.0.2 examples):

```cpp
radio.setSpiPin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_CC1101_CSN); // before Init()
radio.setGDO(PIN_CC1101_GDO0, PIN_CC1101_GDO2);
radio.Init();
radio.setModulation(2);                  // ASK/OOK
radio.setMHZ(433.92);
radio.setPktFormat(3);                   // asynchronous serial mode, FIFO bypassed
radio.SpiWriteReg(CC1101_IOCFG0, 0x0D);  // GDO0 outputs the raw demodulated data
radio.SetRx();
```

**Why.** In packet mode the CC1101 expects a known frame format (sync
word, CRC). Raw capture needs asynchronous serial mode, where GDO0 mirrors
the demodulated signal in real time, like the DATA pin of a cheap 433 MHz
receiver. Both settings are required: `setPktFormat(3)` and
`IOCFG0 = 0x0D`.

**Capture:** a `CHANGE` interrupt on GDO0 records the duration of each
level. ISR rules: `docs/architecture.md`, section 7.

**Done when:**
- [ ] One of the author's own 433.92 MHz remotes produces a repeating
      pattern clearly distinct from background noise. This was never
      confirmed with a real remote in the old firmware.
- [ ] The capture is saved to SD in the format below and can be read back.

### 2.2 File format: Flipper-compatible `.sub`

```
Filetype: Flipper SubGhz RAW File
Version: 1
Frequency: 433920000
Preset: FuriHalSubGhzPresetOok650Async
Protocol: RAW
RAW_Data: 350 -1050 350 -1050 1050 -350 ...
```

- **Positive values** are HIGH durations in µs; **negative values** are LOW
  durations.
- The sign resolves the old firmware's open issue: its files did not record
  whether the signal started HIGH or LOW.
- Confirm the exact format against a real `.sub` file before implementing.

### 2.3 Frequency analyzer

Measure signal strength (RSSI) across frequencies and show where there is
activity, answering "which frequency does my remote use?". Check the exact
RSSI function name in SmartRC 3.0.2 (it was `getRssi()` in older versions).

**Hardware limit:** the module is tuned for 433 MHz, so sensitivity at 315
and 868 MHz is much lower.

### 2.4 Saved signals and replay

A list of the files in `/subghz` and replay of the author's own captures.

**Note:** rolling-code remotes (modern gates, cars) reject replays, and
trying can desynchronize the original remote. Test with the author's own
fixed-code devices (cheap RF sockets, doorbells).

### 2.5 Protocol decoding (later)

Recognize simple fixed-code protocols (Princeton, CAME, Nice) from raw
durations. Not urgent.

---

## [ ] Stage 3 — NFC (PN532)

**Scope:**
1. Card detection: UID, ATQA, SAK and card type.
2. Reading MIFARE Classic sectors on the author's own cards with known
   keys.
3. Saving to SD in a simple text format (UID, type, blocks in hex).
4. Writing to the "magic" cards from the kit (blue fob and white card).
   Identify their exact type first, because different magic card types are
   written differently.

**Accepted limitation:** full card emulation (no physical card) is not
realistic with the PN532, especially for MIFARE Classic. The realistic
flow is read → save → write to a magic card.

**Done when:**
- [ ] UID and type are shown for every card in the kit.
- [ ] A dump saved to SD reads back identically.

---

## [ ] Stage 4 — Infrared

**Scope:** receive and decode (NEC, Samsung, Sony, RC5/RC6, ...), save,
transmit saved signals, and raw capture for unknown protocols.

**Format:** Flipper-compatible `.ir`, so community databases can be used.
Confirm the exact format against a real file before implementing.

```
Filetype: IR signals file
Version: 1
#
name: Power
type: parsed
protocol: NEC
address: 07 00 00 00
command: 02 00 00 00
```

**Library:** see the table in `docs/architecture.md`; the released
IRremoteESP8266 does not build on core 3.x, so the choice is made here.

**Done when:**
- [ ] The author's own TV remote is decoded and saved.
- [ ] The saved signal turns the TV on from ~1 m (range is limited by
      driving the LED straight from the GPIO).

---

## [ ] Stage 5 — BLE (NimBLE-Arduino)

**Scope:**
- **Scanner:** name, address, RSSI and manufacturer (from advertising
  data).
- **Tracker detector:** flags AirTag / SmartTag-style devices seen
  repeatedly nearby. A defensive feature and a good demo.

**Done when:**
- [ ] The author's phone and earbuds appear, with RSSI changing as they
      move closer or further away.

---

## [ ] Stage 6 — Wi-Fi

**Scope:**
- **Network scanner:** SSID, BSSID, channel, RSSI, encryption.
- **Channel monitor:** activity per channel (promiscuous mode, packet
  counts).
- **Attack detector:** flags abnormal numbers of deauthentication or
  disassociation frames. A defensive feature.

Offensive Wi-Fi features are not specified in this document. If they are
ever added, scope and test environment (own, isolated network) are agreed
before any code is written.

**Done when:**
- [ ] The author's own network appears with the correct channel and
      encryption.
- [ ] The monitor shows more activity on the router's channel during a
      download.

---

## [ ] Stage 7 — NRF24

**Prerequisite:** decoupling capacitors soldered (`docs/hardware.md`,
5.1).

**Scope:** a 2.4 GHz band scanner (activity across the NRF24's 126
channels), drawn as a bar graph on the OLED.

**Done when:**
- [ ] The graph clearly shows the author's own Wi-Fi router on the matching
      channels.
- [ ] The module stays stable for minutes without disturbing the SD card or
      the CC1101 on the same bus.

---

## [ ] Stage 8 — USB HID (native USB)

The board enumerates as a keyboard on the **USB** port and runs simple
typing scripts from SD (e.g. open an editor and type a text).

- A script runs only after **explicit on-screen confirmation**.
- Tested only on the author's own computer.
- The hardware is ready: native USB is already wired on the board.

**Done when:**
- [ ] A test script types a known text into an editor on the author's own
      computer.

---

## [ ] Stage 9 — Extensions

- **Battery power** (when the parts arrive), with the startup test from
  `docs/hardware.md` 2.3 and a battery level indicator from the GPIO 3
  divider.
- **125 kHz RFID reading** with the RDM6300 (read-only, serial output;
  needs a free pin: 39 or 47).
- **iButton reading** (DIY probe, one free pin).
- **Enclosure,** following `docs/hardware.md`, section 7.
- **Event sounds:** card detected, signal captured, error.
