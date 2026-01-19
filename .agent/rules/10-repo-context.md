# Repo Context

This repo is PlatformIO-based firmware for LilyGo T-LoRa Pager:
- Build: `pio run`
- Flash: `pio run -t upload`
- Monitor: `pio device monitor` (115200)

Do not invent project structure. Use only paths that exist in this repo:
- src/main.cpp
- src/ui/ui.c, src/ui/ui.h
- variants/.../pins_arduino.h
- boards/...json
- platformio.ini

If unsure, request a file tree scan first.
