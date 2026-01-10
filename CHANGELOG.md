# Changelog

All notable changes to AVERROES T-LoRa Pager firmware.

Format based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## [1.0.0] - 2025-01-10

### Added
- Initial firmware release for LilyGo T-LoRa Pager
- Cyberpunk orange/amber UI theme
- 5-page navigation system:
  - **Home**: Terminal with device info panel
  - **Connectivity**: WiFi, LoRa, Bluetooth, GPS status
  - **Display**: Brightness, theme settings
  - **System**: Sound, haptics, power management
  - **Info**: Device specifications, firmware version
- Status bar with real-time indicators:
  - WiFi signal strength
  - GPS satellite count
  - LoRa radio status
  - Battery percentage with charging indicator
- Input handling:
  - Rotary encoder rotation with haptic feedback
  - Rotary click for selection
  - Rotary long press to return home
  - Orange button (ALT) for page cycling
  - Full QWERTY keyboard support
- Keyboard shortcuts:
  - Keys 1-5: Jump to pages
  - H/N/T/Y/I: Page navigation
- Haptic feedback on all user inputs
- LVGL 9.x graphics with custom styling

### Technical
- PlatformIO build system
- ESP32-S3 Arduino framework
- LilyGoLib hardware abstraction
- Callback-based keyboard input
- Interrupt-driven rotary encoder
- 480x222 landscape display configuration

---

## [Unreleased]

### Planned
- LoRa messaging functionality
- WiFi scanning and connection
- GPS coordinate display
- Theme switching
- Settings persistence

---

## Version History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 2025-01-10 | Initial release with full UI |

---

## Upgrade Notes

### From scratch to 1.0.0
1. Clone repository
2. Run `pio run -t upload`
3. Device ready with full UI
