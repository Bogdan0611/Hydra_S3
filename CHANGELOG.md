# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added

- Project documentation: hardware reference, firmware architecture and
  staged roadmap.
- Contribution guidelines, changelog and C++ formatting configuration.
- PlatformIO project for the ESP32-S3 N16R8 on the pioarduino platform
  (Arduino-ESP32 core 3.3.9).
- GitHub Actions build on every push and pull request.
- Boot log with an `[OK]` / `[ERROR]` line per component, free heap and
  PSRAM, and an error count.
- Core firmware: pin map, I2C/SPI startup, OLED display manager,
  debounced joystick input, non-blocking buzzer, SD card mount with app
  folders, and a reusable scrolling list.
- Main menu with placeholder apps for the later stages.
