# Hardware

Board, power, pinout and module notes. This is the single source of truth
for the hardware: if the code and this file disagree, one of them is wrong
and gets fixed.

---

## 1. Board: ESP32-S3 N16R8

- **Module:** ESP32-S3-WROOM-1 N16R8 — 16 MB flash (QIO), 8 MB octal PSRAM
  (OPI). Standard DevKitC-1 style board, not a "Super Mini".
- **Logic level:** 3.3 V. **GPIOs are not 5 V tolerant.**
- **Two USB-C ports:**
  - **COM** — through a USB-to-UART bridge. Used for flashing and Serial.
  - **USB** — the chip's native USB (GPIO 19 = D-, GPIO 20 = D+), already
    wired on the board. Reserved for the USB HID stage; no extra wiring
    needed.
- **On-board RGB LED (WS2812):** GPIO 48 on some board revisions, GPIO 38
  on others; unknown for this board. If it is on 38 it shares the pin with
  the PN532 IRQ. This is harmless: the LED may flicker while NFC is active.
  No rewiring needed.

---

## 2. Power

### 2.1 Current setup (temporary)

A bench supply set to **3.30 V**, connected directly to the board's **3V3**
pin (and GND).

- **Current limit around 1 A:** enough headroom for peaks, safe on a short.
- **One source at a time:** switch the bench supply off while the board is
  on USB. Two supplies on the same 3V3 node usually cause no harm, but they
  add an unnecessary variable when debugging.
- **USB alone is enough for stage 0,** with a good cable. Long or thin
  cables dropped enough voltage to cause a reset loop at boot
  (`rst:0xf (BROWNOUT_RST)` on Serial) and a flickering screen; a
  medium-length, thick cable fixed it.

### 2.2 Planned setup (parts not arrived)

```
1S LiPo 2000 mAh (103450) → TP4056 with protection (USB-C)
  → slide switch on OUT+ → AP2112K-3.3 (LDO, 600 mA max)
  → board 3V3 pin + all modules
```

- **No 5 V stage anywhere in the device.**
- **LDO capacitors:** 1 µF at the input, 10 µF ceramic at the output,
  100 nF next to the board's 3V3 pin. Bulk capacitance: see 2.3.
- **Dropout.** Below about 3.5 V on the battery the rail sags slightly
  under 3.3 V. The ESP32-S3 is specified down to 3.0 V, so this is
  acceptable.
- **Battery monitoring (later):** 100 kΩ / 100 kΩ divider from the battery
  to **GPIO 3**, the only free ADC1 pin. At 4.2 V the ADC sees 2.1 V.

### 2.3 Bulk capacitors

Available: two to three usable 1000 µF electrolytics, plus a kit up to
470 µF.

| Location | Capacitor | When | Why |
|---|---|---|---|
| NRF24 VCC/GND, soldered at the module pins | **1000 µF** + 100 nF | fitted | The PA draws ~115 mA bursts; the unshielded module is the most supply-sensitive part |
| Main 3V3 rail, at the board's 3V3/GND pins | **1000 µF** + 100 nF | now | Buffers Wi-Fi TX peaks (300 mA+). Same node as the future LDO output |
| CC1101 VCC/GND | 10 µF + 100 nF | before the Sub-GHz stage | Low current (~30 mA TX); a small local cap is enough |
| Third 1000 µF, 470 µF | spare | — | Fallback if a 1000 µF does not fit or fails the check below |

- **Check salvaged capacitors first.** No bulging or leakage; measure the
  capacitance if the multimeter supports it. Any voltage rating ≥ 6.3 V is
  fine on a 3.3 V rail. The stripe marks the negative lead.
- **Inrush is fine today.** Both the bench supply (constant-current limit)
  and the board's own USB regulator charge ~2000 µF in milliseconds.
- **Battery build: startup test required.** The AP2112K has foldback
  short-circuit protection (about 50 mA at 0 V output). Its datasheet warns
  that a load already present at startup can prevent it from starting.
  Around 2000 µF plus the ESP32 load is exactly that situation. When the
  battery chain is built, switch on 10 times in a row: the rail must reach
  3.3 V and the board must boot every time. If it does not, remove the
  main-rail 1000 µF first. The NRF24 keeps its own capacitor.

### 2.4 Current budget (approximate)

| Consumer | Typical | Peak |
|---|---|---|
| ESP32-S3, radios off | 40–80 mA | — |
| ESP32-S3, Wi-Fi active | ~100 mA | 300 mA+ during TX |
| OLED | 10–20 mA | — |
| PN532, RF field on | ~100 mA or more | — |
| CC1101 | RX ~16 mA | TX ~30 mA |
| NRF24L01+PA/LNA | tens of mA | ~115 mA during TX |
| MicroSD | a few mA | ~100 mA while writing |
| IR RX / IR TX / buzzer | ~1 / ~20 / 20–30 mA | — |

Only one radio app runs at a time (see `docs/architecture.md`), so real
consumption stays under the AP2112K's 600 mA. Short peaks are covered by
the bulk capacitors.

---

## 3. Pinout (final)

| GPIO | Signal | Module | Notes |
|---|---|---|---|
| 1 | UP | Joystick | `INPUT_PULLUP`, pressed = LOW |
| 2 | DOWN | Joystick | same |
| 4 | GDO0 | CC1101 | raw data: output in RX, input in TX |
| 5 | LEFT | Joystick | same |
| 6 | RIGHT | Joystick | same |
| 7 | OK (centre) | Joystick | same |
| 8 | SDA | I2C bus | OLED + PN532 |
| 9 | SCL | I2C bus | OLED + PN532 |
| 10 | CSN | CC1101 | SPI chip select |
| 11 | MOSI | SPI bus | CC1101 + NRF24 + SD |
| 12 | SCK | SPI bus | CC1101 + NRF24 + SD |
| 13 | MISO | SPI bus | CC1101 + NRF24 + SD |
| 14 | CSN | NRF24 | SPI chip select |
| 15 | CE | NRF24 | radio enable |
| 16 | IRQ | NRF24 | optional; can be freed |
| 17 | SET (BACK) | Joystick | `INPUT_PULLUP`, pressed = LOW |
| 18 | GDO2 | CC1101 | optional for raw capture; can be freed |
| 21 | CS | MicroSD | SPI chip select (pull-up on the module) |
| 38 | IRQ | PN532 | may be shared with the RGB LED (harmless) |
| 40 | OUT | IR RX | 38 kHz receiver, idles HIGH |
| 41 | S | IR TX | IR LED driven directly by the GPIO |
| 42 | S | Buzzer | 3-pin module |

**Why the pins are good.** GPIO 10–13 are the ESP32-S3's default FSPI
pins, so SPI runs at full speed through the IO MUX, not the GPIO matrix.
No strapping, flash/PSRAM or USB pins are used.

### 3.1 Reserved pins

| GPIO | Reason |
|---|---|
| 0 | BOOT button; strapping |
| 3, 45, 46 | strapping (sampled at reset); 3 is usable as ADC after boot |
| 19, 20 | native USB, reserved for USB HID |
| 26–32 | internal flash (not broken out) |
| 35, 36, 37 | octal PSRAM on R8 modules |
| 43, 44 | UART0, the Serial on the COM port |
| 48 | RGB LED on many boards |

### 3.2 Free pins

Only **39** and **47**, plus **3** (ADC input only, reserved for battery
monitoring). If more are needed, **16** (NRF24 IRQ) and **18** (CC1101
GDO2) are optional for most features and can be freed.

---

## 4. Modules

### 4.1 OLED 0.96" SSD1306 (128×64, I2C)

- **Supply:** accepts 3.3–5 V. **Powered from 3V3**; on some modules the
  I2C pull-ups follow VCC, which would put 5 V on the bus.
- **Address:** `0x3C` (try `0x3D` if the screen stays blank).
- **Header:** VCC, GND, SCL, SDA.

### 4.2 PN532 NFC (13.56 MHz), red "V3" board

- **Supply:** 3.3–5 V (on-board regulator). **Powered from 3V3** so all
  signals stay at 3.3 V.
- **I2C address:** `0x24`.
- **Mode switches:** the silkscreen table gives I2C as **switch 1 = ON,
  switch 2 = OFF**. In any other position the module does not respond at
  all, however correct the wiring.
- **Headers:**
  - **Top (4 pins):** SCL, SDA, VCC, GND — used for I2C.
  - **Right (8 pins):** SCK, MISO, MOSI, SS, VCC, GND, **IRQ**, RSTO —
    only IRQ is used.
  - **Back (4 pins):** GND, VCC, TXD, RXD (HSU mode) — unused.
  - **Side row (holes):** RSTPDN, SIGIN, SIGOUT, SIGCLK, INT1, INT0,
    DBGTXD, DBGRXD, AUX2, AUX1 — unused.
- **RSTO is a reset output, not an input.** Leave it unconnected. The real
  reset input is RSTPDN on the side row; it is not needed.
- **Antenna:** the coil is the PCB itself. Keep metal (battery, ground
  planes) away from directly behind it, or the read range drops sharply.

### 4.3 CC1101 Sub-GHz — blue "433M" module with SMA connector

- **Supply:** 1.8–3.6 V. **3V3 only;** 5 V destroys it, on signal pins
  too.
- **Tuned for 433 MHz.** The "433M" silkscreen means the antenna matching
  network is designed for 433 MHz. Reception and transmission at 315 and
  868 MHz are much weaker; that is expected, not a fault. Test with
  433.92 MHz remotes.
- **2×4 header** (pin 1 = square pad; rows are 1-2, 3-4, 5-6, 7-8):

  | Pin | Signal | | Pin | Signal |
  |---|---|---|---|---|
  | 1 | GND | | 2 | VCC |
  | 3 | GDO0 | | 4 | CSN |
  | 5 | SCK | | 6 | MOSI |
  | 7 | MISO (GDO1) | | 8 | GDO2 |

  This is the standard pinout for this module type. Pin 1 should have
  continuity with the SMA connector body (GND).

### 4.4 NRF24L01+PA/LNA (2.4 GHz) — black, unshielded, SMA connector

- **Supply:** 1.9–3.6 V, **3V3**. The most supply-sensitive module: its PA
  draws ~115 mA bursts during TX.
- **2×4 header** (pin 1 = square pad):

  | Pin | Signal | | Pin | Signal |
  |---|---|---|---|---|
  | 1 | GND | | 2 | VCC |
  | 3 | CE | | 4 | CSN |
  | 5 | SCK | | 6 | MOSI |
  | 7 | MISO | | 8 | IRQ |

  Pin 1 should have continuity with the SMA connector body.
- **Mandatory before any test:** 1000 µF + 100 nF across VCC/GND, soldered
  directly at the module pins (see 2.3). Without them, TX bursts can upset
  the whole SPI bus, including the CC1101 and the SD card.
- **Unshielded:** PA/LNA modules without a metal can may become unstable at
  full power. Start every test at the lowest PA level. Shielding (foil tied
  to GND, insulated from the components) is an optional improvement.
- **The chip may be a clone** (e.g. Si24R1 remarked as nRF24L01+). Basic
  communication works the same; some advanced features may behave
  differently.

### 4.5 MicroSD — simple blue module, no regulator

- **Header (6 pins):** GND, MISO, CLK, MOSI, CS, **3V3**.
- **On board:** the socket, two capacitors and four 10 kΩ pull-ups. No
  regulator and no level shifter.
- **Supply: 3V3, direct.** Ideal for this design: it needs no 5 V, and with
  no level shifter the module itself does not hold the shared MISO line.
  The card does, until it is clocked after deselect (see
  `docs/architecture.md`, section 6.1).
- **CS has a pull-up on the module.**
- **Card:** FAT32 (or exFAT), ideally 32 GB or smaller.

### 4.6 IR RX — 3-pin 38 kHz receiver module (VS1838B type)

- **Supply:** accepts 2.7–5.5 V. **Powered from 3V3,** because the output
  swings to VCC: at 5 V it would put 5 V on the GPIO.
- **Output:** idles HIGH and goes LOW while it receives the 38 kHz
  carrier.

### 4.7 IR TX — 3-pin IR LED module

- **Driven directly by the GPIO,** so the current is limited to ~20 mA and
  the range to about 1–2 m.
- **Optional upgrade:** an NPN transistor (e.g. 2N2222 or BC337) fed from
  the battery, for ~100 mA pulses and a range of several metres.

### 4.8 Buzzer — 3-pin module (VCC, GND, S)

- **Powered from 3V3;** the module's own transistor drives the buzzer, so
  the GPIO only supplies the base current.
- **Active-HIGH,** identified at first boot: silent with the pin LOW,
  clicks on every key press (`BUZZER_ACTIVE_HIGH = true` in `config.h`).
  Whether the buzzer itself is active or passive is still unknown; the PWM
  drive works for both.

### 4.9 5-way joystick + SET button

- **Passive switches,** no supply. COM to GND.
- **Pins:** all `INPUT_PULLUP`; pressed = LOW.

---

## 5. Hardware work

### 5.1 To do (in order)

1. **Main rail:** 1000 µF + 100 nF at the board's 3V3/GND pins.
2. **NRF24:** 1000 µF + 100 nF at the module pins. **Done.**
3. **CC1101:** 10 µF + 100 nF at the module pins.
4. **Optional:** 10 kΩ pull-ups on CS 10 and CS 14, so no SPI module is
   selected while the ESP32 boots. The firmware drives all CS pins HIGH
   before SPI init anyway.
5. **Optional:** NRF24 shielding; transistor driver for IR TX.
6. **When the parts arrive:** the battery chain from 2.2, the startup test
   from 2.3, and the battery divider on GPIO 3.

### 5.2 Deferred (missing hardware, not code)

- **125 kHz RFID with writing:** the RDM6300, the only module available
  locally, reads only. An EM4095-based module has to be ordered.
- **iButton / 1-Wire probe:** two contacts and a ~4.7 kΩ pull-up. Can be
  built any time; needs a free pin (39 or 47).

---

## 6. First-boot checks

The "Hardware Test" app (stage 1 in `docs/roadmap.md`) automates these.
Until then, the expected results are:

| Check | Pass | If it fails |
|---|---|---|
| Power | no `rst:0xf (BROWNOUT_RST)` on Serial at boot | supply too weak → shorter, thicker USB cable (see 2.1) |
| I2C scan | `0x3C` (OLED) and `0x24` (PN532) found | PN532 missing → mode switches; nothing found → SDA/SCL swapped |
| MicroSD | mounts; card type and size printed | card not FAT32, wrong CS, MISO wire |
| CC1101 | VERSION register reads `0x14` (older chips: `0x04`) | `0x00`/`0xFF` → no SPI communication: CSN, MISO, supply |
| NRF24 | `isChipConnected()` returns true | supply, capacitors, CSN/CE |
| IR RX | idles HIGH; edges appear when a remote is pressed | VCC/GND swapped, wrong pin |
| Buzzer | see below | — |
| RGB LED | — | if it flickers while NFC is active, it is on GPIO 38 (harmless) |

**Identifying the buzzer:** drive the pin HIGH for one second, then LOW
for one second.

- **Continuous tone on HIGH:** active buzzer, active-HIGH.
- **Continuous tone on LOW:** active buzzer, active-LOW.
- **Only a click:** passive buzzer; it needs a PWM signal (`tone()`).

---

## 7. Physical layout (enclosure / PCB)

- **PN532 on the back,** with nothing metallic behind the coil.
- **Antennas at the edges, as far apart as possible:** CC1101, NRF24 and
  the ESP32 PCB antenna. Never cover the ESP32 antenna with metal.
- **Battery at the bottom,** away from the NFC coil and the antennas.
- **Decoupling capacitors at the module pins,** not at the far end of the
  wires.
- **Short SPI wires;** lower the SPI clock in firmware if long wires cause
  errors.
- **OLED and joystick on the front.**
