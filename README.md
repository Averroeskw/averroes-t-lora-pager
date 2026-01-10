# AVERROES T-LoRa Pager Firmware

Custom cyberpunk-themed firmware for the LilyGo T-LoRa Pager device with full LVGL UI.

## Hardware

- **Device**: LilyGo T-LoRa Pager
- **MCU**: ESP32-S3 @ 240MHz
- **Flash**: 16MB
- **PSRAM**: 8MB QSPI
- **Display**: ST7796 480x222 (Landscape)
- **Radio**: SX1262 LoRa
- **Input**: QWERTY Keyboard + Rotary Encoder

## Features

- Cyberpunk orange/amber theme
- 5-page navigation system (Home, Connectivity, Display, System, Info)
- Terminal interface with device info panel
- Full keyboard input with shortcuts
- Rotary encoder with haptic feedback
- Orange button page switching
- Status bar with WiFi, GPS, LoRa, Battery indicators

---

## Build Environment Setup

### Prerequisites

1. **Python 3.8+** - Required for PlatformIO
2. **PlatformIO Core** - Build system for embedded development

### Install PlatformIO

```bash
# Install PlatformIO Core
pip install platformio

# Or using Homebrew (macOS)
brew install platformio
```

### Verify Installation

```bash
pio --version
# Should output: PlatformIO Core, version X.X.X
```

---

## Libraries (Auto-installed)

PlatformIO automatically downloads these dependencies on first build:

| Library | Version | Purpose |
|---------|---------|---------|
| [LilyGoLib](https://github.com/Xinyuan-LilyGO/LilyGoLib) | latest | Board support, hardware abstraction |
| [LVGL](https://github.com/lvgl/lvgl) | ^9.4.0 | Graphics library for UI |
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson) | ^7.0.3 | JSON parsing |
| [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus) | ^1.0.3 | GPS NMEA parsing |
| [Adafruit TCA8418](https://github.com/adafruit/Adafruit_TCA8418) | ^1.0.2 | Keyboard controller |
| [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO) | ^1.17.0 | I2C/SPI abstraction |
| NFC-RFAL-fork | latest | NFC support |
| ST25R3916-fork | latest | NFC reader IC |

---

## Building the Firmware

### 1. Clone the Repository

```bash
git clone https://github.com/Averroeskw/averroes-t-lora-pager.git
cd averroes-t-lora-pager
```

### 2. Build Only (No Upload)

```bash
pio run
```

First build takes 2-5 minutes (downloads dependencies + compiles).
Subsequent builds are much faster (~30 seconds).

### 3. Build Output

Successful build shows:
```
RAM:   [=         ]   8.1% (used 26556 bytes from 327680 bytes)
Flash: [===       ]  29.6% (used 930489 bytes from 3145728 bytes)
========================= [SUCCESS] Took XX.XX seconds =========================
```

---

## Flashing the Firmware

### Connect the Device

1. Connect T-LoRa Pager via USB-C
2. Device appears as `/dev/cu.usbmodemXXX` (macOS) or `COM#` (Windows)

### Flash Command

```bash
pio run -t upload
```

### Manual Port Selection (if needed)

```bash
pio run -t upload --upload-port /dev/cu.usbmodem101
```

### Flash Output

Successful flash shows:
```
Writing at 0x000f30a1... (100 %)
Wrote 930848 bytes (596608 compressed) at 0x00010000 in 7.5 seconds
Hash of data verified.
Hard resetting via RTS pin...
========================= [SUCCESS] Took XX.XX seconds =========================
```

---

## Serial Monitor

View debug output and logs:

```bash
pio device monitor
```

Or with specific port:

```bash
pio device monitor --port /dev/cu.usbmodem101 --baud 115200
```

---

## Controls

| Input | Action |
|-------|--------|
| **Orange Button (ALT)** | Cycle through pages |
| **Rotary Rotation** | Scroll/Navigate + haptic |
| **Rotary Click** | Select current item |
| **Rotary Long Press** | Return to Home page |
| **Enter Key** | Select |
| **Keys 1-5** | Jump to page 1-5 |
| **H** | Home page |
| **N** | Connectivity page |
| **T** | Display page |
| **Y** | System page |
| **I** | Info page |

---

## Project Structure

```
averroes-t-lora-pager/
├── README.md                 # This file
├── CLAUDE.md                 # AI assistant guidelines
├── TODO.md                   # Permanent task tracking
├── CHANGELOG.md              # Version history
├── platformio.ini            # Build configuration
├── boards/
│   └── lilygo-t-lora-pager.json    # Board definition
├── variants/
│   └── lilygo_tlora_pager/
│       └── pins_arduino.h    # Pin mappings
├── src/
│   ├── main.cpp              # Main firmware entry
│   └── ui/
│       ├── ui.h              # UI header & definitions
│       └── ui.c              # UI implementation
└── docs/                     # Additional documentation
```

---

## Troubleshooting

### Build Errors

**"Board not found"**
```bash
# Ensure boards directory exists with JSON file
ls boards/lilygo-t-lora-pager.json
```

**"Library not found"**
```bash
# Force library reinstall
pio pkg install --force
```

### Upload Errors

**"Failed to connect"**
1. Unplug and replug USB cable
2. Hold BOOT button while connecting
3. Try different USB port/cable

**"Permission denied" (Linux/macOS)**
```bash
sudo chmod 666 /dev/ttyUSB0  # Linux
sudo chmod 666 /dev/cu.usbmodem101  # macOS
```

### Display Issues

**"No display output"**
- Check brightness setting in `setup()`: `instance.setBrightness(200);`
- Verify LVGL is initialized: Check serial output for `[INIT] LVGL... OK`

---

## License

Private repository - AVERROES proprietary firmware.

---

## Author

**AVERROES** - Custom T-LoRa Pager Firmware
