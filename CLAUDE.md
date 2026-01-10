# CLAUDE.md - AI Assistant Guidelines

This file provides context and rules for AI assistants (Claude, etc.) working on this codebase.

---

## Project Overview

**AVERROES T-LoRa Pager Firmware** - Custom ESP32-S3 firmware for LilyGo T-LoRa Pager with cyberpunk UI.

### Tech Stack
- **Platform**: ESP32-S3 (Arduino Framework)
- **Build System**: PlatformIO
- **UI Library**: LVGL 9.x
- **Hardware Library**: LilyGoLib (provides global `instance` reference)

---

## Critical Rules

### 1. Never Create Duplicate Files
- Always check if a file exists before creating
- Prefer editing existing files over creating new ones
- One source of truth per component

### 2. Keep Codebase Clean
- Remove unused code completely (no commented-out code blocks)
- No backwards-compatibility shims unless explicitly requested
- Delete unused variables, don't rename to `_unused`

### 3. Preserve Working Functionality
- Always test builds after changes: `pio run`
- If something works, don't refactor it unless asked
- User prefers callback-based keyboard over polling

### 4. Character Encoding
- **NO Unicode box-drawing characters** - the font doesn't support them
- Use ASCII: `+`, `-`, `|` for boxes instead of `╔`, `═`, `╚`
- Use `LV_SYMBOL_*` macros for icons (they render correctly)
- Avoid: `◎`, `◐`, `ℹ`, `█`, `▪`, `►` - use LV_SYMBOL equivalents

### 5. Input Handling Preferences
- **Keyboard**: Use callback-based approach (`instance.kb.setCallback()`)
- **Orange Button**: Raw callback for ALT key (0x14)
- **Encoder Direction**: Original direction (b != a) ? 1 : -1 is correct

---

## File Purposes

| File | Purpose | Modify When |
|------|---------|-------------|
| `src/main.cpp` | Entry point, input handling, hardware init | Adding new hardware features |
| `src/ui/ui.h` | UI definitions, colors, macros, declarations | Adding new UI components/pages |
| `src/ui/ui.c` | UI implementation, page layouts, widgets | Changing UI appearance/behavior |
| `platformio.ini` | Build config, libraries, flags | Adding libraries or build options |
| `boards/*.json` | Board hardware definition | Never (hardware-specific) |
| `variants/*/pins_arduino.h` | Pin mappings | Never (hardware-specific) |

---

## Common Tasks

### Adding a New Page

1. Add to `ui_page_t` enum in `ui.h`
2. Declare `ui_screen_newpage` extern in `ui.h`
3. Declare `ui_setup_newpage_page(void)` in `ui.h`
4. Implement in `ui.c`
5. Add to switch statement in `ui_load_page()`
6. Update `PAGE_COUNT`

### Changing Theme Colors

Edit the `#define` values in `ui.h`:
```c
#define UI_COLOR_PRIMARY        lv_color_hex(0xFF6B00)
```

### Adding Keyboard Shortcuts

Edit `ui_handle_key_press()` in `ui.c`:
```c
case 'X':
    ui_load_page(PAGE_SOMETHING);
    break;
```

---

## Build Commands

```bash
# Build only
pio run

# Build and flash
pio run -t upload

# Serial monitor
pio device monitor

# Clean build
pio run -t clean
```

---

## Common Pitfalls

### 1. LVGL 9.x vs 8.x API
- Use `lv_obj_set_style_*` (v9) not `lv_style_set_*` on objects directly
- Screen loading: `lv_scr_load_anim(screen, anim, time, delay, false)`

### 2. Memory
- ESP32-S3 has 8MB PSRAM - allocate large buffers there if needed
- LVGL uses internal memory management

### 3. Hardware Access
- Always use `instance.` prefix for LilyGoLib functions
- Example: `instance.setBrightness(200)`, `instance.vibrator()`

---

## Git Workflow

### Commit Messages
Format: `type: short description`

Types:
- `feat:` New feature
- `fix:` Bug fix
- `ui:` UI changes
- `refactor:` Code restructure
- `docs:` Documentation
- `build:` Build config changes

### Before Committing
1. Build succeeds: `pio run`
2. No compiler warnings
3. Update CHANGELOG.md
4. Update TODO.md if tasks completed

---

## Contact

Repository maintained by AVERROES.
