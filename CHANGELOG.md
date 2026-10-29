# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

## [0.1.0] - 2026-10-29

Stage 0: Foundation.

### Added

- Project documentation: hardware reference, firmware architecture and
  staged roadmap.
- Contribution guidelines, changelog and C++ formatting configuration.
- PlatformIO project for the ESP32-S3 N16R8 on the pioarduino platform
  (Arduino-ESP32 core 3.3.9).
- GitHub Actions build on every pull request and every push to `main`.
- Boot log with an `[OK]` / `[ERROR]` line per component, free heap and
  PSRAM, and an error count.
- Core firmware: pin map, I2C/SPI startup, OLED display manager,
  debounced joystick input, non-blocking buzzer, SD card mount with app
  folders, and a reusable scrolling list.
- Main menu with placeholder apps for the later stages.
- On-screen notice when boot finds no SD card or another error.

### Fixed

- SD card mounted only intermittently at 10 MHz on jumper wires; the SPI
  clock is now 4 MHz.

[Unreleased]: https://github.com/Bogdan0611/Hydra_S3/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/Bogdan0611/Hydra_S3/releases/tag/v0.1.0
