/**
 * AVERROES T-LoRa Pager - Full UI Firmware
 * Cyberpunk Theme with Orange/Amber Accent
 * Resolution: 480x222 (Horizontal)
 *
 * Control Scheme:
 *   - Orange Button (ALT key): Page Switcher (cycles pages)
 *   - Rotary Rotation: Scroll/Navigate with haptic
 *   - Rotary Press: Select
 *   - Rotary Long Press: Back to Home
 *   - Enter: Select
 *   - Keyboard: Full QWERTY input
 */

#include <Arduino.h>
#include <LilyGoLib.h>
#include <LV_Helper.h>
#include <WiFi.h>
#include "ui/ui.h"

// LilyGoLib provides global 'instance' reference

// Encoder state (interrupt-driven)
static volatile int encPos = 0;
static int lastEncPos = 0;

// Button debounce
static uint32_t lastButtonTime = 0;
static bool lastButtonState = false;

// Long press detection
static uint32_t buttonPressStart = 0;
static bool longPressHandled = false;
#define LONG_PRESS_MS 800

// Orange button (ALT key) detection
// ALT key raw code = 0x14 (Row 2, Col 0 in keyboard matrix)
#define KEY_ALT_RAW 0x14
static volatile bool orangeButtonPressed = false;
static uint32_t lastOrangeButtonTime = 0;
#define ORANGE_BUTTON_DEBOUNCE_MS 200

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

// Raw keyboard callback - catches ALT key (orange button) before processing
void onRawKey(bool pressed, uint8_t raw) {
    // Only handle orange button (ALT key 0x14) on press with debounce
    if (raw == KEY_ALT_RAW && pressed) {
        uint32_t now = millis();
        if (now - lastOrangeButtonTime > ORANGE_BUTTON_DEBOUNCE_MS) {
            lastOrangeButtonTime = now;
            orangeButtonPressed = true;
            Serial.println("[ORANGE] Page switch triggered!");
        }
    }
}

// Keyboard callback for character input
void onKeyPress(int state, char &c) {
    if (state == 1) {  // KB_PRESSED
        Serial.printf("[KEY] Pressed: '%c' (0x%02X)\n", c, (uint8_t)c);

        // Pass to UI handler
        ui_handle_key_press((uint8_t)c);

        // Haptic feedback
        instance.setHapticEffects(1);
        instance.vibrator();
    }
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n========================================");
    Serial.println("   AVERROES T-LoRa Pager v1.0");
    Serial.println("========================================");

    // Initialize board hardware
    Serial.print("[INIT] Board... ");
    instance.begin();
    Serial.println("OK");

    // Initialize LVGL display
    Serial.print("[INIT] LVGL... ");
    beginLvglHelper(instance);
    Serial.println("OK");

    // Set brightness
    instance.setBrightness(200);

    // Setup rotary encoder pins
    pinMode(ROTARY_A, INPUT_PULLUP);
    pinMode(ROTARY_B, INPUT_PULLUP);
    pinMode(ROTARY_C, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ROTARY_A), encISR, CHANGE);
    Serial.println("[INIT] Encoder... OK");

    // Setup keyboard callbacks
    instance.kb.setRawCallback(onRawKey);      // Catch orange button
    instance.kb.setCallback(onKeyPress);        // Catch character input
    instance.kb.setRepeat(true);                // Enable key repeat
    instance.kb.setBrightness(200);             // Keyboard backlight
    Serial.println("[INIT] Keyboard... OK");

    // Initialize the full UI system
    Serial.print("[INIT] UI System... ");
    ui_init();
    Serial.println("OK");

    // Startup haptic feedback
    instance.setHapticEffects(47);
    instance.vibrator();

    Serial.println("========================================");
    Serial.println("   System Ready!");
    Serial.println("   Controls:");
    Serial.println("   - Orange btn: Switch pages");
    Serial.println("   - Rotary: Scroll, Click=Select");
    Serial.println("   - Long press: Home");
    Serial.println("   - Enter: Select");
    Serial.println("========================================\n");
}

void loop() {
    // Process LilyGoLib tasks (handles keyboard polling internally)
    instance.loop();

    // Handle orange button (page switcher) - set by raw callback
    if (orangeButtonPressed) {
        orangeButtonPressed = false;

        // Cycle through pages: 0 -> 1 -> 2 -> 3 -> 4 -> 0 ...
        ui_page_t nextPage = (ui_page_t)((ui_state.currentPage + 1) % PAGE_COUNT);
        Serial.printf("[PAGE] Switching to page %d\n", nextPage);
        ui_load_page_anim(nextPage, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        // Haptic feedback for page switch
        instance.setHapticEffects(47);
        instance.vibrator();
    }

    // Handle encoder rotation
    int pos = encPos;
    int delta = pos - lastEncPos;
    if (delta != 0) {
        lastEncPos = pos;
        Serial.printf("[ENC] Delta: %d\n", delta);
        ui_handle_encoder_rotate((int8_t)delta);

        // Haptic feedback on rotation
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
            if (!longPressHandled) {
                // Short click = Select
                Serial.println("[ENC] Click -> Select");
                ui_handle_encoder_click();
                instance.setHapticEffects(7);
                instance.vibrator();
            }
        }
    }

    // Check for long press while button is held
    if (isPressed && !longPressHandled && (now - buttonPressStart) > LONG_PRESS_MS) {
        longPressHandled = true;
        Serial.println("[ENC] Long Press -> Home");
        ui_handle_encoder_long_press();
        instance.setHapticEffects(14);
        instance.vibrator();
    }

    // Update battery status periodically
    static uint32_t lastBattUpdate = 0;
    if (now - lastBattUpdate > 2000) {
        lastBattUpdate = now;

        uint8_t pct = (uint8_t)instance.gauge.getStateOfCharge();
        bool charging = instance.ppm.isCharging();
        ui_update_battery(pct, charging);
    }

    // Process LVGL tasks
    lv_timer_handler();
    delay(5);
}
