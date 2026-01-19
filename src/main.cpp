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

  // Create SSH Terminal
  sshTerminal = new SSHTerminal();
  terminalScreen = sshTerminal->create_terminal_screen();
  lv_scr_load(terminalScreen);

  // Display startup message
  sshTerminal->append_text("AVERROES SSH Terminal v1.0\n");
  sshTerminal->append_text("Type 'help' for commands\n\n");
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

  // Handle encoder rotation -> Navigate command history
  int pos = encPos;
  int delta = pos - lastEncPos;
  if (delta != 0 && sshTerminal) {
    lastEncPos = pos;
    Serial.printf("[ENC] Delta: %d\n", delta);

    // Navigate history (up = older, down = newer)
    sshTerminal->navigate_history(delta > 0 ? 1 : -1);

    // Haptic feedback
    instance.setHapticEffects(3);
    instance.vibrator();
  }

  // Handle encoder button with debounce and long press
  bool isPressed = !digitalRead(ROTARY_C);
  uint32_t now = millis();

  if (isPressed != lastButtonState && (now - lastButtonTime) > 50) {
    lastButtonTime = now;
    lastButtonState = isPressed;

    if (isPressed) {
      // Button pressed - start timing for long press
      buttonPressStart = now;
      longPressHandled = false;
    } else {
      // Button released
      if (!longPressHandled && sshTerminal) {
        // Short click = Enter
        Serial.println("[ENC] Click -> Enter");
        sshTerminal->handle_key_input('\n');
        instance.setHapticEffects(7);
        instance.vibrator();
      }
    }
  }

  // Check for long press while button is held
  if (isPressed && !longPressHandled &&
      (now - buttonPressStart) > LONG_PRESS_MS) {
    longPressHandled = true;
    if (sshTerminal) {
      Serial.println("[ENC] Long Press -> Delete History Entry");
      sshTerminal->delete_current_history_entry();
      instance.setHapticEffects(14);
      instance.vibrator();
    }
  }

  // Update battery status periodically
  static uint32_t lastBattUpdate = 0;
  if (now - lastBattUpdate > 5000 && sshTerminal) {
    lastBattUpdate = now;
    sshTerminal->update_status_bar();
  }

  // Process LVGL tasks
  lv_timer_handler();
  delay(5);
}
