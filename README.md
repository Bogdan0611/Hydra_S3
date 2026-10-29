# ESP32-S3 Hardware Multi-Tool

A handheld hardware-security multi-tool in the spirit of Flipper Zero, built
from discrete modules around an ESP32-S3. It covers Sub-GHz, NFC, infrared
and 2.4 GHz, and adds what the ESP32-S3 brings on its own: native Wi-Fi,
BLE and USB, a dual-core 240 MHz CPU and 8 MB of PSRAM.

> **Status:** work in progress — Stage 0 (Foundation). See the
> [roadmap](docs/roadmap.md).

## Features

| Area | Module | Planned capabilities | Status |
|---|---|---|---|
| Sub-GHz | CC1101 | Raw capture, frequency analyzer, saved signals, protocol decoding | Planned |
| NFC | PN532 | Card detection, MIFARE Classic read, dump to SD | Planned |
| Infrared | 38 kHz RX + IR LED | Receive, decode, save and transmit | Planned |
| BLE | ESP32-S3 | Scanner, tracker detector | Planned |
| Wi-Fi | ESP32-S3 | Network scanner, channel monitor, attack detector | Planned |
| 2.4 GHz | NRF24L01+PA/LNA | Band scanner | Planned |
| USB HID | ESP32-S3 native USB | Scripted keyboard input from SD | Planned |

Signals are stored on the SD card in Flipper-compatible text formats where
possible (`.sub`, `.ir`).

## Hardware

| Component | Interface | Notes |
|---|---|---|
| ESP32-S3-WROOM-1 N16R8 dev board | — | 16 MB flash, 8 MB octal PSRAM, dual USB-C |
| SSD1306 0.96" OLED (128×64) | I2C | Shared bus with the PN532 |
| PN532 NFC module | I2C | 13.56 MHz |
| CC1101 module | SPI | Tuned for 433 MHz, SMA antenna |
| NRF24L01+PA/LNA | SPI | 2.4 GHz, SMA antenna |
| MicroSD module | SPI | 3.3 V, no level shifter |
| IR receiver + IR LED | GPIO | 38 kHz |
| 5-way joystick + button | GPIO | Navigation |
| Buzzer module | GPIO | Feedback |

Everything runs at 3.3 V. Full pinout, power design and module notes:
[docs/hardware.md](docs/hardware.md).

## Building

Requirements: [VS Code](https://code.visualstudio.com/) with the
PlatformIO IDE extension. The project uses the
[pioarduino](https://github.com/pioarduino/platform-espressif32) platform
fork (Arduino-ESP32 core 3.x), pinned in `platformio.ini`; it is downloaded
automatically on the first build.

```bash
pio run                        # build
pio run -t upload              # flash (USB-C port labelled "COM")
pio device monitor             # serial monitor, 115200 baud
```

## Documentation

- [Hardware](docs/hardware.md) — board, power, pinout, modules, first-boot checks
- [Architecture](docs/architecture.md) — firmware structure and design decisions
- [Roadmap](docs/roadmap.md) — stages, acceptance criteria, technical notes
- [Contributing](CONTRIBUTING.md) — branching, commit conventions, releases
- [Changelog](CHANGELOG.md)

## Responsible use

This project is for learning and for authorized testing only. Use it
exclusively on devices, cards and networks you own or are explicitly
permitted to test. Transmitting radio signals may be regulated where you
live; you are responsible for complying with local law.
