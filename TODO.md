# TODO - AVERROES T-LoRa Pager

Permanent task tracking for the project. Update this file as tasks are completed.

---

## Completed

### v1.0.0 (2025-01-10)

- [x] Initial firmware setup with PlatformIO
- [x] Board definition for T-LoRa Pager (ESP32-S3)
- [x] LVGL 9.x integration
- [x] Cyberpunk orange/amber theme implementation
- [x] 5-page navigation system (Home, Connectivity, Display, System, Info)
- [x] Status bar with WiFi, GPS, LoRa, Battery indicators
- [x] Rotary encoder support with ISR
- [x] Rotary button with short click and long press detection
- [x] Orange button (ALT key) page cycling
- [x] Keyboard callback-based input handling
- [x] Haptic feedback on all inputs
- [x] Home page: Terminal + Device Info two-column layout
- [x] Keyboard shortcuts (1-5 pages, H/N/T/Y/I navigation)
- [x] Fix Unicode characters - replaced with LV_SYMBOL and ASCII
- [x] Repository setup with clean structure

---

## In Progress

- [ ] LoRa messaging functionality
- [ ] WiFi connectivity and scanning
- [ ] GPS coordinate display

---

## Planned Features

### Communication
- [ ] LoRa peer-to-peer messaging
- [ ] LoRa message history storage
- [ ] WiFi AP mode for configuration
- [ ] WiFi client mode for internet
- [ ] Bluetooth connectivity

### UI Enhancements
- [ ] Theme switcher (Cyan, Magenta, Matrix Green, Amber Retro)
- [ ] Brightness slider on Display page
- [ ] Animated transitions between pages
- [ ] Message notification popup
- [ ] Keyboard LED patterns

### System Features
- [ ] Settings persistence (NVS storage)
- [ ] OTA firmware updates
- [ ] SD card file browser
- [ ] Power management / sleep modes
- [ ] Real-time clock sync from GPS

### GPS Features
- [ ] Live coordinate display
- [ ] Distance/bearing to saved waypoints
- [ ] Track logging
- [ ] Geofencing alerts

### NFC Features
- [ ] NFC tag reading
- [ ] NFC tag writing
- [ ] Contact sharing via NFC

---

## Known Issues

| Issue | Status | Notes |
|-------|--------|-------|
| None currently | - | - |

---

## Ideas / Wishlist

- Morse code input mode
- QR code display for sharing
- Mini games (snake, tetris)
- Audio alerts with patterns
- Custom boot animation
- External antenna indicator

---

## Notes

- Always update this file when completing tasks
- Move completed items to the Completed section with date
- Add version number when releasing
