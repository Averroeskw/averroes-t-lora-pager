/**
 * AVERROES T-LoRa Pager - SSH Terminal Firmware
 * Ported from PocketSSH (https://github.com/0015/PocketSSH)
 * Adapted for LilyGo T-LoRa-Pager hardware
 *
 * Control Scheme:
 *   - Rotary Rotation: Navigate command history
 *   - Rotary Press: Execute current input (Enter)
 *   - Rotary Long Press: Delete current history entry
 *   - Keyboard: Full QWERTY input
 */

#include "ssh/ssh_terminal.h"
#include <Arduino.h>
#include <LV_Helper.h>
#include <LilyGoLib.h>

// Global SSH Terminal instance
static SSHTerminal *sshTerminal = nullptr;
static lv_obj_t *terminalScreen = nullptr;

// Encoder state (interrupt-driven)
static volatile int encPos = 0;
static int lastEncPos = 0;

// Button debounce
static uint32_t lastButtonTime = 0;
static bool lastButtonState = false;

// Long press detection
static uint32_t buttonPressStart = 0;
static bool longPressHandled = false;
#define LONG_PRESS_MS 1000

// Encoder ISR
void IRAM_ATTR encISR() {
  static int lastA = 0;
  int a = digitalRead(ROTARY_A);
  int b = digitalRead(ROTARY_B);
  if (a != lastA) {
    encPos += (b != a) ? 1 : -1;
  }
  lastA = a;
}

// Custom Keyboard Config for TCA8418
static const char keymap[4][10] = {
    {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
    {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\n'},
    {'\0', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '\0', '\0'},
    {' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}};
static const char symbol_map[4][10] = {
    {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
    {'*', '/', '+', '-', '=', ':', '\'', '"', '@', '\0'},
    {'\0', '_', '$', ';', '?', '!', ',', '.', '\0', '\0'},
    {' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}};
static const LilyGoKeyboardConfigure_t myKeyboardConfig = {
    .kb_rows = 4,
    .kb_cols = 10,
    .current_keymap = (char *)&keymap[0][0],
    .current_symbol_map = (char *)&symbol_map[0][0],
    .symbol_key_value = 0x14, // Orange Button
    .alt_key_value = 0xFF,    // Disable generic Alt
    .caps_key_value = 0x1C,
    .caps_b_key_value = 0xFF,
    .char_b_value = 0x19,
    .backspace_value = 0x1D,
    .has_symbol_key = true};

// Keyboard callback for characters
void onKeyPress(int state, char &c) {
  if (state == 1 && sshTerminal) { // KB_PRESSED
    // Haptic feedback
    instance.setHapticEffects(1);
    instance.vibrator();

    // Pass to SSH Terminal
    Serial.printf("[KEY] %c (0x%02X)\n", c, c);
    sshTerminal->handle_key_input(c);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("AVERROES SSH TERMINAL BOOTING...");

  // Initialize LilyGoLib
  instance.begin();

  // Set brightness
  instance.setBrightness(150);

  // Setup keyboard callback
  instance.kb.setCallback(onKeyPress);

  // Initialize keyboard with custom config
  Serial.println("[DEBUG] Initializing Keyboard...");
  if (instance.kb.begin(myKeyboardConfig, Wire, KB_INT, SDA, SCL)) {
    Serial.println("[DEBUG] Keyboard Init SUCCESS");
  } else {
    Serial.println("[DEBUG] Keyboard Init FAILED");
  }

  // Initialize Display & LVGL
  beginLvglHelper(instance);

  // Initialize SSH Terminal
  sshTerminal = new SSHTerminal();
  terminalScreen = sshTerminal->create_terminal_screen();
  lv_obj_t *launcherScreen = sshTerminal->create_launcher_screen();

  // Start with launcher screen
  sshTerminal->show_launcher();

  // Display startup message (will be visible when user enters terminal)
  sshTerminal->append_text("═══════════════════════════════════════\n");
  sshTerminal->append_text("  AVERROES SSH Terminal v2.1\n");
  sshTerminal->append_text("  Secure Shell Client for ESP32\n");
  sshTerminal->append_text("═══════════════════════════════════════\n\n");
  sshTerminal->append_text("Quick Start:\n");
  sshTerminal->append_text("  1. connect <WiFi> <Pass>\n");
  sshTerminal->append_text("  2. save local <host> <port> <user> <pass>\n");
  sshTerminal->append_text("  3. home - goes back to launcher\n\n");
  sshTerminal->update_status_bar();

  // Hardware interrupts for encoder
  pinMode(ROTARY_A, INPUT_PULLUP);
  pinMode(ROTARY_B, INPUT_PULLUP);
  pinMode(ROTARY_C, INPUT_PULLUP);
  attachInterrupt(ROTARY_A, encISR, CHANGE);

  Serial.println("System Ready.");
}

void loop() {
  // System Loop (Keyboard, etc)
  instance.loop();

  // Handle encoder rotation
  int pos = encPos;
  int delta = pos - lastEncPos;
  if (delta != 0 && sshTerminal) {
    lastEncPos = pos;

    // Haptic feedback - use effect 1 (strong click) for snappiness
    instance.setHapticEffects(1);
    instance.vibrator();

    if (sshTerminal->is_in_launcher()) {
      // Navigate launcher buttons
      lv_group_t *g = sshTerminal->get_launcher_group();
      if (delta > 0)
        lv_group_focus_next(g);
      else
        lv_group_focus_prev(g);
    } else {
      // Navigate history in terminal - allow proportional scrolling
      sshTerminal->navigate_history(delta);
    }
  }

  // Handle encoder button with debounce and long press
  bool isPressed = !digitalRead(ROTARY_C);
  uint32_t now = millis();

  if (isPressed != lastButtonState && (now - lastButtonTime) > 50) {
    lastButtonTime = now;
    lastButtonState = isPressed;

    if (isPressed) {
      buttonPressStart = now;
      longPressHandled = false;
      instance.setHapticEffects(1);
      instance.vibrator();
    } else {
      // Released
      if (!longPressHandled && sshTerminal) {
        if (sshTerminal->is_in_launcher()) {
          // Select current profile
          lv_group_t *g = sshTerminal->get_launcher_group();
          lv_obj_t *focused = lv_group_get_focused(g);
          if (focused) {
            const char *type = (const char *)lv_obj_get_user_data(focused);
            sshTerminal->show_terminal();
            sshTerminal->connect_to_profile(type);
          }
        } else {
          // Send Enter to terminal
          sshTerminal->handle_key_input('\n');
        }
        instance.setHapticEffects(7);
        instance.vibrator();
      }
    }
  }

  // Handle long press (only in terminal)
  if (isPressed && !longPressHandled && !sshTerminal->is_in_launcher() &&
      (now - buttonPressStart) > LONG_PRESS_MS) {
    longPressHandled = true;
    sshTerminal->delete_current_history_entry();
    instance.setHapticEffects(14);
    instance.vibrator();
  }

  // Update status bar periodically
  static uint32_t lastBattUpdate = 0;
  if (now - lastBattUpdate > 5000 && sshTerminal) {
    lastBattUpdate = now;
    sshTerminal->update_status_bar();
  }

  // Process LVGL tasks
  lv_timer_handler();
  delay(5);
}
