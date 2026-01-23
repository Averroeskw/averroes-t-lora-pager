/**
 * SSH Terminal for AVERROES Pager
 * Ported from PocketSSH (https://github.com/0015/PocketSSH)
 * Adapted for LilyGo T-LoRa-Pager hardware
 *
 * Theme: Cyberpunk Yellow - Vibrant Yellow on Black with White highlights
 */

#include "ssh_terminal.h"
#include "../hal/void_hal.h"
#include <LilyGoLib.h>
#include <Preferences.h>
#include <esp_timer.h>
#include <stdlib.h>

static const char *TAG = "SSH_TERMINAL";
static Preferences preferences;
SSHTerminal *SSHTerminal::ssht_instance = nullptr;

// Access to the global LilyGo instance for vibration
extern LilyGoLoRaPager &instance;

// ═══════════════════════════════════════════════════════════════════════════
// CYBERPUNK YELLOW THEME
// ═══════════════════════════════════════════════════════════════════════════
#define COLOR_BG lv_color_hex(0x000000)        // Pure Black
#define COLOR_FG lv_color_hex(0xFFDD00)        // Vibrant Yellow (primary)
#define COLOR_HIGHLIGHT lv_color_hex(0xFFFFFF) // White (highlights)
#define COLOR_DIM lv_color_hex(0x665500)       // Dim Yellow (muted)
#define COLOR_ACCENT lv_color_hex(0xFFAA00)    // Orange-Yellow (accent)
#define COLOR_SUCCESS lv_color_hex(0x00FF66)   // Neon Green
#define COLOR_ERROR lv_color_hex(0xFF3333)     // Bright Red
#define COLOR_WARNING lv_color_hex(0xFFFF00)   // Pure Yellow
#define COLOR_SHELL lv_color_hex(0x00FFB3)     // Matrix Green/Cyan

// UI Constants
#define STATUS_BAR_HEIGHT 28
#define INPUT_BAR_HEIGHT 32
#define CORNER_RADIUS 8 // Rounded corners for premium feel

SSHTerminal::SSHTerminal() {
  ssht_instance = this;
  load_history();
  load_session();
}

SSHTerminal::~SSHTerminal() {
  save_history();
  save_session();
  disconnect();
  wifi_disconnect();
}

lv_obj_t *SSHTerminal::create_terminal_screen() {
  terminal_screen = lv_obj_create(NULL);
  lv_obj_set_size(terminal_screen, 240, 320); // CRITICAL: Fix screen scaling
  lv_obj_set_style_bg_color(terminal_screen, COLOR_BG, 0);
  lv_obj_set_style_pad_all(terminal_screen, 0, 0);

  // ═══════════════════════════════════════════════════════════════════════
  // HEADER/STATUS BAR - Vibrant Yellow background, black text
  // ═══════════════════════════════════════════════════════════════════════
  lv_obj_t *header_bar = lv_obj_create(terminal_screen);
  lv_obj_set_size(header_bar, LV_PCT(100), STATUS_BAR_HEIGHT);
  lv_obj_align(header_bar, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_set_style_bg_color(header_bar, COLOR_FG, 0);
  lv_obj_set_style_bg_opa(header_bar, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(header_bar, 0, 0);
  lv_obj_set_style_radius(header_bar, 0, 0);
  lv_obj_set_style_pad_hor(header_bar, 8, 0);
  lv_obj_set_style_pad_ver(header_bar, 4, 0);
  lv_obj_set_style_shadow_width(header_bar, 12, 0);
  lv_obj_set_style_shadow_color(header_bar, COLOR_FG, 0);
  lv_obj_set_style_shadow_opa(header_bar, 150, 0);
  lv_obj_clear_flag(header_bar, LV_OBJ_FLAG_SCROLLABLE);

  // Title label (left side)
  lv_obj_t *title_label = lv_label_create(header_bar);
  lv_obj_set_style_text_color(title_label, COLOR_BG, 0);
  lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
  lv_label_set_text(title_label, LV_SYMBOL_HOME " AVERROES SSH");
  lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 0, 0);

  // Status indicator (right side)
  status_bar = lv_label_create(header_bar);
  lv_obj_set_style_text_color(status_bar, COLOR_BG, 0);
  lv_obj_set_style_text_font(status_bar, &lv_font_montserrat_14, 0);
  lv_label_set_text(status_bar, LV_SYMBOL_WIFI " OFF");
  lv_obj_align(status_bar, LV_ALIGN_RIGHT_MID, 0, 0);

  // Byte counter (next to status)
  byte_counter_label = lv_label_create(header_bar);
  lv_obj_set_style_text_color(byte_counter_label, COLOR_BG, 0);
  lv_obj_set_style_text_font(byte_counter_label, &lv_font_montserrat_12, 0);
  lv_label_set_text(byte_counter_label, "");
  lv_obj_align(byte_counter_label, LV_ALIGN_RIGHT_MID, -80, 0);

  // ═══════════════════════════════════════════════════════════════════════
  // OUTPUT AREA - Main terminal content, scrollable
  // ═══════════════════════════════════════════════════════════════════════
  lv_obj_t *output_container = lv_obj_create(terminal_screen);
  lv_obj_set_pos(output_container, 0, STATUS_BAR_HEIGHT);
  // Explicitly set height to fill middle area
  lv_obj_set_size(output_container, LV_PCT(100),
                  320 - STATUS_BAR_HEIGHT - INPUT_BAR_HEIGHT);
  lv_obj_set_style_bg_color(output_container, COLOR_BG, 0);

  // Background Pattern for terminal (Subtle points or grid)
  for (int i = 0; i < 240; i += 20) {
    lv_obj_t *v_line = lv_obj_create(output_container);
    lv_obj_set_size(v_line, 1, 320);
    lv_obj_set_pos(v_line, i, 0);
    lv_obj_set_style_bg_color(v_line, COLOR_FG, 0);
    lv_obj_set_style_bg_opa(v_line, 10, 0);
    lv_obj_set_style_border_width(v_line, 0, 0);
    lv_obj_add_flag(v_line, LV_OBJ_FLAG_IGNORE_LAYOUT);
  }
  for (int i = 0; i < 320; i += 20) {
    lv_obj_t *h_line = lv_obj_create(output_container);
    lv_obj_set_size(h_line, 240, 1);
    lv_obj_set_pos(h_line, 0, i);
    lv_obj_set_style_bg_color(h_line, COLOR_FG, 0);
    lv_obj_set_style_bg_opa(h_line, 10, 0);
    lv_obj_set_style_border_width(h_line, 0, 0);
    lv_obj_add_flag(h_line, LV_OBJ_FLAG_IGNORE_LAYOUT);
  }

  lv_obj_set_style_border_width(output_container, 0, 0);
  lv_obj_set_style_radius(output_container, 0, 0);
  lv_obj_set_style_pad_all(output_container, 8, 0);
  lv_obj_set_scroll_dir(output_container, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(output_container, LV_SCROLLBAR_MODE_AUTO);

  output_label = lv_label_create(output_container);
  lv_obj_set_width(output_label, LV_PCT(100));
  lv_obj_set_style_text_color(output_label, COLOR_FG, 0);
  lv_obj_set_style_text_font(output_label, &lv_font_montserrat_14,
                             0); // Clarity increase
  lv_obj_set_style_text_line_space(output_label, 4,
                                   0);          // Decouple "squished" lines
  lv_obj_set_style_pad_hor(output_label, 5, 0); // Internal margin
  lv_label_set_long_mode(output_label, LV_LABEL_LONG_WRAP);
  lv_label_set_text(output_label, "");

  // ═══════════════════════════════════════════════════════════════════════
  // INPUT BAR - Bottom command line, white text on dim background
  // ═══════════════════════════════════════════════════════════════════════
  lv_obj_t *input_bar = lv_obj_create(terminal_screen);
  lv_obj_set_size(input_bar, LV_PCT(100), INPUT_BAR_HEIGHT);
  lv_obj_align(input_bar, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_bg_color(input_bar, COLOR_DIM, 0);
  lv_obj_set_style_bg_opa(input_bar, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(input_bar, COLOR_FG, 0);
  lv_obj_set_style_border_width(input_bar, 2, 0);
  lv_obj_set_style_border_side(input_bar, LV_BORDER_SIDE_TOP, 0);
  lv_obj_set_style_radius(input_bar, 0, 0);
  lv_obj_set_style_pad_hor(input_bar, 8, 0);
  lv_obj_set_style_pad_ver(input_bar, 4, 0);
  lv_obj_set_style_shadow_width(input_bar, 12, 0);
  lv_obj_set_style_shadow_color(input_bar, COLOR_FG, 0);
  lv_obj_set_style_shadow_opa(input_bar, 100, 0);
  lv_obj_clear_flag(input_bar, LV_OBJ_FLAG_SCROLLABLE);

  // Prompt symbol
  lv_obj_t *prompt_label = lv_label_create(input_bar);
  lv_obj_set_style_text_color(prompt_label, COLOR_FG, 0);
  lv_obj_set_style_text_font(prompt_label, &lv_font_montserrat_16, 0);
  lv_label_set_text(prompt_label, ">");
  lv_obj_align(prompt_label, LV_ALIGN_LEFT_MID, 0, 0);

  // Input text (white for visibility)
  input_label = lv_label_create(input_bar);
  lv_obj_set_style_text_color(input_label, COLOR_HIGHLIGHT, 0); // White!
  lv_obj_set_style_text_font(input_label, &lv_font_montserrat_14, 0);
  lv_label_set_text(input_label, "|");
  lv_obj_align(input_label, LV_ALIGN_LEFT_MID, 16, 0);

  // Blinking cursor animation
  lv_anim_t a_cursor;
  lv_anim_init(&a_cursor);
  lv_anim_set_var(&a_cursor, input_label);
  lv_anim_set_exec_cb(&a_cursor, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
  lv_anim_set_values(&a_cursor, 255, 0);
  lv_anim_set_time(&a_cursor, 500);
  lv_anim_set_playback_time(&a_cursor, 500);
  lv_anim_set_repeat_count(&a_cursor, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a_cursor);

  return terminal_screen;
}

lv_obj_t *SSHTerminal::create_launcher_screen() {
  launcher_screen = lv_obj_create(NULL);
  lv_obj_set_size(launcher_screen, 240, 320); // CRITICAL: Fix screen scaling
  lv_obj_set_style_bg_color(launcher_screen, COLOR_BG, 0);

  // 1. INFINITY WIREFRAME GRID (Base Layer)
  lv_obj_t *grid_container = lv_obj_create(launcher_screen);
  lv_obj_set_size(grid_container, 240, 320);
  lv_obj_align(grid_container, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_opa(grid_container, 0, 0);
  lv_obj_set_style_border_width(grid_container, 0, 0);
  lv_obj_clear_flag(grid_container, LV_OBJ_FLAG_SCROLLABLE);

  for (int i = 0; i < 15; i++) {
    lv_obj_t *line = lv_obj_create(grid_container);
    lv_obj_set_size(line, 240, 1);
    lv_obj_set_style_bg_color(line, COLOR_FG, 0);
    lv_obj_set_style_bg_opa(line, 12, 0);
    lv_obj_set_style_border_width(line, 0, 0);

    lv_anim_t a_grid;
    lv_anim_init(&a_grid);
    lv_anim_set_var(&a_grid, line);
    lv_anim_set_exec_cb(&a_grid, (lv_anim_exec_xcb_t)grid_scroll_anim_cb);
    lv_anim_set_values(&a_grid, 0, 320);
    lv_anim_set_time(&a_grid, 5000);
    lv_anim_set_delay(&a_grid, i * 333);
    lv_anim_set_repeat_count(&a_grid, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a_grid);
  }

  // 2. VIGNETTE EFFECT (Second Layer)
  lv_obj_t *vignette = lv_obj_create(launcher_screen);
  lv_obj_set_size(vignette, 240, 320);
  lv_obj_set_style_bg_opa(vignette, 0, 0);
  lv_obj_set_style_border_width(vignette, 0, 0);
  lv_obj_set_style_shadow_width(vignette, 120, 0);
  lv_obj_set_style_shadow_color(vignette, lv_color_hex(0x000000), 0);
  lv_obj_set_style_shadow_opa(vignette, 230,
                              0); // Slightly lighter for visibility
  // lv_obj_set_style_shadow_spread(vignette, 10, 0);
  lv_obj_add_flag(vignette, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_ext_click_area(vignette, 0);

  // 3. UI TITLES
  lv_obj_t *title = lv_label_create(launcher_screen);
  lv_obj_set_style_text_color(title, COLOR_HIGHLIGHT, 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_24,
                             0); // Bold digital look
  lv_label_set_text(title, "VOID-GATE");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);

  // START FLICKER ANIMATION
  lv_anim_t a_flicker;
  lv_anim_init(&a_flicker);
  lv_anim_set_var(&a_flicker, title);
  lv_anim_set_exec_cb(&a_flicker, (lv_anim_exec_xcb_t)title_flicker_cb);
  lv_anim_set_values(&a_flicker, 0, 255);
  lv_anim_set_time(&a_flicker, 800);
  lv_anim_set_playback_time(&a_flicker, 50);
  lv_anim_set_repeat_count(&a_flicker, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a_flicker);

  lv_obj_t *subtitle = lv_label_create(launcher_screen);
  lv_obj_set_style_text_color(subtitle, COLOR_DIM, 0);
  lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_12, 0);
  lv_label_set_text(subtitle, "ENCRYPTION NODE v5.1"); // Creative Branding
  lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 70);

  // 4. NAVIGATION GROUP & STYLES
  launcher_group = lv_group_create();

  static lv_style_transition_dsc_t trans_btn;
  static lv_style_prop_t trans_props[] = {LV_STYLE_BORDER_WIDTH,
                                          LV_STYLE_BORDER_COLOR,
                                          LV_STYLE_SHADOW_WIDTH,
                                          LV_STYLE_TEXT_COLOR,
                                          LV_STYLE_TRANSFORM_SCALE_X,
                                          LV_STYLE_TRANSFORM_SCALE_Y,
                                          0};
  lv_style_transition_dsc_init(&trans_btn, trans_props, lv_anim_path_ease_out,
                               200, 0, NULL);

  static lv_style_t style_btn, style_btn_focused;
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, COLOR_BG);
  lv_style_set_bg_opa(&style_btn, 180);
  lv_style_set_border_color(&style_btn, COLOR_DIM);
  lv_style_set_border_width(&style_btn, 1);
  lv_style_set_radius(&style_btn, CORNER_RADIUS);
  lv_style_set_text_color(&style_btn, COLOR_DIM);
  lv_style_set_transition(&style_btn, &trans_btn);

  lv_style_init(&style_btn_focused);
  lv_style_set_border_color(&style_btn_focused, COLOR_FG);
  lv_style_set_border_width(&style_btn_focused, 4);
  lv_style_set_shadow_color(&style_btn_focused, COLOR_FG);
  lv_style_set_shadow_width(&style_btn_focused, 30);
  lv_style_set_shadow_opa(&style_btn_focused, 220);
  lv_style_set_text_color(&style_btn_focused, COLOR_HIGHLIGHT);
  lv_style_set_transform_scale(&style_btn_focused, 285);

  // 5. SIDE-BY-SIDE BUTTONS
  lv_obj_t *btn_local = lv_btn_create(launcher_screen);
  lv_obj_set_size(btn_local, 100, 70); // Adjusted for better center clearance
  lv_obj_align(btn_local, LV_ALIGN_CENTER, -55, 45);
  lv_obj_add_style(btn_local, &style_btn, 0);
  lv_obj_add_style(btn_local, &style_btn_focused, LV_STATE_FOCUSED);
  lv_obj_t *label_local = lv_label_create(btn_local);
  lv_label_set_text(label_local,
                    LV_SYMBOL_HOME "\nLOCAL"); // Newline for side-by-side
  lv_obj_set_style_text_align(label_local, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label_local);
  lv_obj_set_user_data(btn_local, (void *)"local");
  lv_group_add_obj(launcher_group, btn_local);

  lv_obj_t *btn_remote = lv_btn_create(launcher_screen);
  lv_obj_set_size(btn_remote, 100, 70);
  lv_obj_align(btn_remote, LV_ALIGN_CENTER, 55, 45);
  lv_obj_add_style(btn_remote, &style_btn, 0);
  lv_obj_add_style(btn_remote, &style_btn_focused, LV_STATE_FOCUSED);
  lv_obj_t *label_remote = lv_label_create(btn_remote);
  lv_label_set_text(label_remote, LV_SYMBOL_WIFI "\nREMOTE");
  lv_obj_set_style_text_align(label_remote, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label_remote);
  lv_obj_set_user_data(btn_remote, (void *)"remote");
  lv_group_add_obj(launcher_group, btn_remote);

  // 6. CRT OVERLAY (Top Layer)
  for (int i = 0; i < 320; i += 3) {
    lv_obj_t *scanline = lv_obj_create(launcher_screen);
    lv_obj_set_size(scanline, 240, 1);
    lv_obj_align(scanline, LV_ALIGN_TOP_MID, 0, i);
    lv_obj_set_style_bg_color(scanline, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(scanline, 20,
                            0); // Much lighter scanlines for better text
    lv_obj_set_style_border_width(scanline, 0, 0);
    lv_obj_add_flag(scanline, LV_OBJ_FLAG_IGNORE_LAYOUT);
  }
  return launcher_screen;
}

void SSHTerminal::show_launcher() {
  lv_async_call(async_show_launcher_cb, NULL);
}

void SSHTerminal::async_show_launcher_cb(void *param) {
  if (!ssht_instance || !ssht_instance->launcher_screen)
    return;
  lvgl_lock();
  ssht_instance->in_launcher = true;
  lv_scr_load(ssht_instance->launcher_screen);
  lvgl_unlock();
}

void SSHTerminal::show_terminal() {
  lv_async_call(async_show_terminal_cb, NULL);
}

void SSHTerminal::async_show_terminal_cb(void *param) {
  if (!ssht_instance || !ssht_instance->terminal_screen)
    return;
  lvgl_lock();
  lv_screen_load_anim(ssht_instance->terminal_screen, LV_SCR_LOAD_ANIM_FADE_ON,
                      200, 0, false);
  lv_group_focus_obj(ssht_instance->output_label);
  ssht_instance->in_launcher = false;
  lvgl_unlock();
}

void SSHTerminal::append_text(const char *text) {
  if (!text)
    return;

  // Decouple from background tasks using pooled messages
  UIMessage *msg = ssht_instance->ui_queue.checkout();
  if (!msg)
    return;

  strncpy(msg->text, text, MAX_ASYNC_TEXT - 1);
  msg->text[MAX_ASYNC_TEXT - 1] = '\0';
  msg->clear = false;
  lv_async_call(async_append_text_cb, msg);
}

void SSHTerminal::async_append_text_cb(void *param) {
  UIMessage *msg = (UIMessage *)param;
  if (!msg || !ssht_instance || !ssht_instance->output_label) {
    if (msg)
      ssht_instance->ui_queue.release(msg);
    return;
  }

  lvgl_lock();
  ssht_instance->cumulative_text += msg->text;

  // Keep terminal buffer within reasonable limits (4KB)
  if (ssht_instance->cumulative_text.size() > 4096) {
    ssht_instance->cumulative_text = ssht_instance->cumulative_text.substr(
        ssht_instance->cumulative_text.size() - 2048);
  }

  lv_label_set_text(ssht_instance->output_label,
                    ssht_instance->cumulative_text.c_str());

  // Scroll to bottom
  lv_obj_scroll_to_y(lv_obj_get_parent(ssht_instance->output_label),
                     LV_COORD_MAX, LV_ANIM_ON);
  lvgl_unlock();

  ssht_instance->ui_queue.release(msg);
}

void SSHTerminal::update_status_bar() {
  char *buf = (char *)malloc(64);
  if (!buf)
    return;

  // Refresh and get battery info via HAL
  VOID_HAL::refreshPower();
  float volt = VOID_HAL::getBatteryVoltage();
  int percent = VOID_HAL::getBatteryPercent();

  const char *wifi_status = wifi_connected ? "ONLINE" : "OFFLINE";
  if (is_connecting)
    wifi_status = "BUSY...";

  snprintf(buf, 64,
           "#FFD700 " LV_SYMBOL_BATTERY_3
           " %d%% (%.2fV) #  #00FF00 " LV_SYMBOL_WIFI " %s #",
           percent, volt, wifi_status);

  // Pass formatted string to main thread
  lv_async_call(async_update_status_cb, buf);
}

void SSHTerminal::async_update_status_cb(void *param) {
  char *buf = (char *)param;
  if (!buf || !ssht_instance || !ssht_instance->status_bar) {
    if (buf)
      free(buf);
    return;
  }

  lvgl_lock();
  lv_label_set_text(ssht_instance->status_bar, buf);
  lvgl_unlock();

  free(buf);
}

void SSHTerminal::async_update_counter_cb(void *param) {
  char *buf = (char *)param;
  if (!buf || !ssht_instance || !ssht_instance->byte_counter_label) {
    if (buf)
      free(buf);
    return;
  }
  lvgl_lock();
  lv_label_set_text(ssht_instance->byte_counter_label, buf);
  lvgl_unlock();
  free(buf);
}

void SSHTerminal::update_input_display() {
  std::string *input_ptr = new std::string("> " + current_input);
  lv_async_call(async_update_input_cb, input_ptr);
}

void SSHTerminal::async_update_input_cb(void *param) {
  std::string *input = (std::string *)param;
  if (!input || !ssht_instance || !ssht_instance->input_label) {
    if (input)
      delete input;
    return;
  }
  lvgl_lock();
  lv_label_set_text(ssht_instance->input_label, input->c_str());
  lvgl_unlock();
  delete input;
}

void SSHTerminal::clear_terminal() {
  lvgl_lock();
  cumulative_text.clear();
  lv_label_set_text(output_label, "");
  lvgl_unlock();
}

void SSHTerminal::vibrate(uint32_t ms) {
  VOID_HAL::vibrate(1); // Standard click
}

void SSHTerminal::launcher_event_cb(lv_event_t *e) {
  const char *type = (const char *)lv_event_get_user_data(e);
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  if (ssht_instance) {
    lvgl_lock();
    ssht_instance->trigger_glitch(lv_scr_act()); // Glitch the whole screen!
    ssht_instance->vibrate(60);
    ssht_instance->connect_to_profile(type);
    lvgl_unlock();
  }
}

void SSHTerminal::save_wg_config(const char *private_key,
                                 const char *public_key, const char *endpoint,
                                 const char *local_ip) {
  preferences.begin("wg", false);
  preferences.putString("priv", private_key);
  preferences.putString("pub", public_key);
  preferences.putString("end", endpoint);
  preferences.putString("ip", local_ip);
  preferences.end();
}

bool SSHTerminal::load_wg_config(WireGuardConfig &config) {
  preferences.begin("wg", true);
  config.private_key = preferences.getString("priv", "").c_str();
  config.remote_public_key = preferences.getString("pub", "").c_str();
  config.endpoint = preferences.getString("end", "").c_str();
  config.local_ip = preferences.getString("ip", "").c_str();
  preferences.end();

  // Fallback to hardcoded defaults if NVS is empty
  if (config.private_key.empty()) {
    config.private_key = "iJxJ5aP8FjseZBH6kD7teEh16pb8GycpsHuA+3+b1XQ=";
    config.remote_public_key = "mk24FS+YDTEQ4QwUOWj00dXGdng0WRQz3w5dC9FZkyw=";
    config.endpoint = "188.70.11.115:51820";
    config.local_ip = "10.0.0.2";
  }

  // Parse endpoint port if present (host:port)
  size_t colon = config.endpoint.find(':');
  if (colon != std::string::npos) {
    config.port = atoi(config.endpoint.substr(colon + 1).c_str());
    config.endpoint = config.endpoint.substr(0, colon);
  } else {
    config.port = 51820; // Default WG port
  }

  return true;
}

void SSHTerminal::save_profile(const char *type, const char *host, int port,
                               const char *user, const char *pass) {
  std::string key = std::string("prof_") + type;
  preferences.begin(key.c_str(), false);
  preferences.putString("host", host);
  preferences.putInt("port", port);
  preferences.putString("user", user);
  preferences.putString("pass", pass);
  preferences.end();
}

bool SSHTerminal::load_profile(const char *type, SSHProfile &profile) {
  std::string key = std::string("prof_") + type;
  preferences.begin(key.c_str(), true);

  // Try to load from NVS
  if (preferences.isKey("host")) {
    profile.host = preferences.getString("host", "").c_str();
    profile.port = preferences.getInt("port", 22);
    profile.user = preferences.getString("user", "").c_str();
    profile.pass = preferences.getString("pass", "").c_str();
    preferences.end();
    return true;
  }
  preferences.end();

  // Fallback to hardcoded defaults if NVS is empty
  if (strcmp(type, "local") == 0) {
    profile.host = "192.168.1.10";
    profile.port = 22;
    profile.user = "archie";
    profile.pass = "archie";
    return true;
  } else if (strcmp(type, "remote") == 0) {
    profile.host = "10.0.0.1";
    profile.port = 22;
    profile.user = "archie";
    profile.pass = "archie";
    return true;
  }

  return false;
}

void SSHTerminal::connect_to_profile(const char *type) {
  if (!type)
    return; // Safety check

  if (is_connecting) {
    append_text("[BUSY] Connection in progress...\n");
    return;
  }

  if (ssh_connected) {
    append_text("[INFO] Already connected. Type 'exit' first.\n");
    return;
  }

  is_connecting = true;
  vibrate(100);

  // Use a heap-allocated copy to pass to the task safely
  char *task_type = strdup(type);
  if (!task_type) {
    is_connecting = false;
    return;
  }

  // Increase stack to 16KB for stable SSH negotiation
  xTaskCreate(connection_task, "ssh_connect", 1024 * 16, (void *)task_type, 5,
              &connection_task_handle);
}

void SSHTerminal::connection_task(void *param) {
  char *type_ptr = (char *)param;
  std::string type = type_ptr;
  free(type_ptr); // Free the copy immediately after stringifying

  SSHTerminal *ui = ssht_instance;

  ui->show_terminal();
  ui->append_text("\n[INIT] Requesting Secure Session...\n");

  SSHProfile prof;
  if (ui->load_profile(type.c_str(), prof)) {
    if (!ui->wifi_connected) {
      if (!ui->wifi_auto_connect()) {
        ui->append_text("[ERROR] Link Offline. Check WiFi.\n");
        ui->is_connecting = false;
        ui->connection_task_handle = nullptr;
        vTaskDelete(NULL);
        return;
      }
    }

    if (type == "remote") {
      if (!ui->wg_connect()) {
        ui->append_text("[ERROR] Tunnel Failure. Aborting.\n");
        ui->is_connecting = false;
        ui->connection_task_handle = nullptr;
        vTaskDelete(NULL);
        return;
      }
    }

    ui->append_text("Negotiating SSH Handshake...\n");
    bool success = ui->connect(prof.host.c_str(), prof.port, prof.user.c_str(),
                               prof.pass.c_str());
    if (!success) {
      ui->append_text("[ERROR] SSH Negotiation Failed.\n");
    }
  } else {
    ui->append_text("[ERROR] Profile Not Found.\n");
  }

  ui->is_connecting = false;
  ui->connection_task_handle = nullptr;
  vTaskDelete(NULL);
}

bool SSHTerminal::wg_connect() {
  WireGuardConfig config;
  if (!load_wg_config(config)) {
    append_text("Error: WireGuard config not found.\n");
    return false;
  }

  append_text("Establishing WireGuard Tunnel...\n");

  IPAddress local_ip;
  if (!local_ip.fromString(config.local_ip.c_str())) {
    append_text("Error: Invalid WireGuard Local IP.\n");
    return false;
  }

  append_text("Configuring tunnel (MTU 1420)...\n");

  if (wg.begin(local_ip, config.private_key.c_str(), config.endpoint.c_str(),
               config.remote_public_key.c_str(), config.port)) {

    // HANDSHAKE VERIFICATION LOOP
    append_text("Waiting for Handshake...");
    int retry = 0;
    while (!wg.is_initialized() && retry < 10) {
      append_text(".");
      vTaskDelay(pdMS_TO_TICKS(500));
      retry++;
    }

    if (wg.is_initialized()) {
      append_text("\n[SUCCESS] WireGuard Tunnel active.\n");
      return true;
    } else {
      append_text("\n[FAILED] Handshake timeout.\n");
      return false;
    }
  } else {
    append_text("WireGuard initialization failed!\n");
    return false;
  }
}

void SSHTerminal::wg_disconnect() {
  wg.end();
  append_text("WireGuard Tunnel DOWN.\n");
}

bool SSHTerminal::wifi_connect(const char *ssid, const char *password) {
  append_text("Connecting to WiFi: ");
  append_text(ssid);
  append_text("\n");

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    append_text(".");
    attempts++;
  }
  append_text("\n");

  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    append_text("WiFi connected! IP: ");
    append_text(WiFi.localIP().toString().c_str());
    append_text("\n");

    // RESTORE NTP SYNCHRONIZATION
    append_text("Synchronizing Time (NTP)...");
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);
    int time_wait = 0;
    while (now < 10000 && time_wait < 15) {
      delay(500);
      now = time(nullptr);
      append_text(".");
      time_wait++;
    }

    if (now > 10000) {
      append_text("\nTime Synced.\n");
    } else {
      append_text("\nTime Sync Failed (Handshake may fail).\n");
    }

    update_status_bar();

    // Save WiFi credentials for auto-connect
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", password);
    preferences.end();

    return true;
  } else {
    append_text("WiFi connection failed!\n");
    return false;
  }
}

bool SSHTerminal::wifi_auto_connect() {
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  preferences.end();

  if (ssid != "") {
    return wifi_connect(ssid.c_str(), pass.c_str());
  }

  // Fallback to defaults
  append_text("Trying default WiFi (AAF)...\n");
  return wifi_connect("AAF", "12345678");
}

void SSHTerminal::wifi_disconnect() {
  if (wifi_connected) {
    WiFi.disconnect(true);
    wifi_connected = false;
    update_status_bar();
  }
}

bool SSHTerminal::connect(const char *host, int port, const char *user,
                          const char *pass) {
  if (!wifi_connected) {
    append_text("Error: WiFi not connected!\n");
    return false;
  }

  append_text("Connecting to SSH: ");
  append_text(host);
  append_text(":");
  char port_str[8];
  snprintf(port_str, sizeof(port_str), "%d", port);
  append_text(port_str);
  append_text("\n");

  // Initialize SSH session
  session = ssh_new();
  if (!session) {
    append_text("Failed to create SSH session!\n");
    return false;
  }

  ssh_options_set(session, SSH_OPTIONS_HOST, host);
  ssh_options_set(session, SSH_OPTIONS_PORT, &port);
  ssh_options_set(session, SSH_OPTIONS_USER, user);

  // Set explicit timeout (10 seconds)
  long timeout_sec = 10;
  ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeout_sec);

  // Connect
  int rc = ssh_connect(session);
  if (rc != SSH_OK) {
    append_text("SSH connection failed: ");
    append_text(ssh_get_error(session));
    append_text("\n");

    // Help the user diagnose
    if (strstr(host, "100.")) {
      append_text("Tip: Tailscale IPs (100.x.x.x) only work if this WiFi has a "
                  "Tailscale Subnet Router.\n");
    } else if (strncmp(host, "192.168.", 8) == 0) {
      append_text("Tip: Check if your local machine (192.168.x.x) is on the "
                  "same WiFi (AAF).\n");
    }

    ssh_free(session);
    session = nullptr;
    return false;
  }

  // Authenticate
  rc = ssh_userauth_password(session, nullptr, pass);
  if (rc != SSH_AUTH_SUCCESS) {
    append_text("SSH authentication failed!\n");
    ssh_disconnect(session);
    ssh_free(session);
    session = nullptr;
    return false;
  }

  // Open channel
  channel = ssh_channel_new(session);
  if (!channel) {
    append_text("Failed to create channel!\n");
    ssh_disconnect(session);
    ssh_free(session);
    session = nullptr;
    return false;
  }

  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK) {
    append_text("Failed to open channel!\n");
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);
    channel = nullptr;
    session = nullptr;
    return false;
  }

  // Request PTY
  rc = ssh_channel_request_pty_size(channel, "vt100", 80, 24);
  if (rc != SSH_OK) {
    append_text("Failed to request PTY!\n");
  }

  // Start shell
  rc = ssh_channel_request_shell(channel);
  if (rc != SSH_OK) {
    append_text("Failed to start shell!\n");
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);
    channel = nullptr;
    session = nullptr;
    return false;
  }

  ssh_connected = true;
  append_text("SSH connected!\n");
  update_status_bar();

  // Start receive task
  run_receive_task = true;
  xTaskCreate(ssh_receive_task, "ssh_rx", 1024 * 16, this, 5,
              &receive_task_handle);

  return true;
}

void SSHTerminal::disconnect() {
  run_receive_task = false;       // Signal receive task to exit
  vTaskDelay(pdMS_TO_TICKS(100)); // Give it time to exit gracefully
  receive_task_handle = nullptr;

  if (channel) {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    channel = nullptr;
  }

  if (session) {
    ssh_disconnect(session);
    ssh_free(session);
    session = nullptr;
  }

  ssh_connected = false;
  update_status_bar();
  append_text("SSH disconnected.\n");
}

void SSHTerminal::send_command(const char *cmd) {
  if (!ssh_connected || !channel)
    return;

  ssh_channel_write(channel, cmd, strlen(cmd));
}

// Send special control characters (Ctrl+C, Tab, etc.)
void SSHTerminal::send_special_key(uint8_t key_code) {
  if (!ssh_connected || !channel)
    return;

  char buf[2] = {(char)key_code, 0};
  ssh_channel_write(channel, buf, 1);
}

void SSHTerminal::handle_key_input(char key) {
  if (key == '\n' || key == '\r') {
    // Process command
    if (ssh_connected) {
      // Send to SSH
      current_input += '\n';
      send_command(current_input.c_str());
    } else {
      // Local command processing
      append_text("\n");

      if (current_input.rfind("connect ", 0) == 0) {
        // Parse: connect SSID PASSWORD
        size_t first_space = current_input.find(' ');
        size_t second_space = current_input.find(' ', first_space + 1);
        if (second_space != std::string::npos) {
          std::string ssid = current_input.substr(
              first_space + 1, second_space - first_space - 1);
          std::string password = current_input.substr(second_space + 1);
          wifi_connect(ssid.c_str(), password.c_str());
        } else {
          append_text("Usage: connect <SSID> <PASSWORD>\n");
        }
      } else if (current_input.rfind("ssh ", 0) == 0) {
        // Parse: ssh HOST PORT USER PASS
        std::vector<std::string> parts;
        size_t pos = 0;
        std::string temp = current_input;
        while ((pos = temp.find(' ')) != std::string::npos) {
          parts.push_back(temp.substr(0, pos));
          temp.erase(0, pos + 1);
        }
        parts.push_back(temp);

        if (parts.size() >= 5) {
          connect(parts[1].c_str(), atoi(parts[2].c_str()), parts[3].c_str(),
                  parts[4].c_str());
        } else {
          append_text("Usage: ssh <HOST> <PORT> <USER> <PASS>\n");
        }
      } else if (current_input.rfind("save ", 0) == 0) {
        // Parse: save <local|remote> <host> <port> <user> <pass>
        std::vector<std::string> parts;
        size_t pos = 0;
        std::string temp = current_input;
        while ((pos = temp.find(' ')) != std::string::npos) {
          parts.push_back(temp.substr(0, pos));
          temp.erase(0, pos + 1);
        }
        parts.push_back(temp);

        if (parts.size() >= 6) {
          save_profile(parts[1].c_str(), parts[2].c_str(),
                       atoi(parts[3].c_str()), parts[4].c_str(),
                       parts[5].c_str());
          append_text("Profile [");
          append_text(parts[1].c_str());
          append_text("] saved.\n");
        } else {
          append_text(
              "Usage: save <local|remote> <host> <port> <user> <pass>\n");
        }
      } else if (current_input.rfind("save wg ", 0) == 0) {
        // Parse: save wg <priv> <pub> <endpoint:port> <local_ip>
        std::vector<std::string> parts;
        size_t pos = 0;
        std::string temp = current_input;
        while ((pos = temp.find(' ')) != std::string::npos) {
          parts.push_back(temp.substr(0, pos));
          temp.erase(0, pos + 1);
        }
        parts.push_back(temp);

        if (parts.size() >= 6) {
          save_wg_config(parts[2].c_str(), parts[3].c_str(), parts[4].c_str(),
                         parts[5].c_str());
          append_text("WireGuard config saved.\n");
        } else {
          append_text("Usage: save wg <priv> <pub> <end:port> <ip>\n");
        }
      } else if (current_input == "disconnect") {
        wifi_disconnect();
        append_text("WiFi disconnected.\n");
      } else if (current_input == "exit") {
        disconnect();
      } else if (current_input == "clear") {
        clear_terminal();
      } else if (current_input == "settings") {
        show_launcher();
        return; // Skip history for settings navigation
      } else if (current_input == "neofetch") {
        display_neofetch_banner();
      } else if (current_input == "help") {
        append_text("Commands:\n");
        append_text("  connect <SSID> <PASS> - Connect WiFi\n");
        append_text("  ssh <H> <P> <U> <P> - Manual SSH\n");
        append_text("  save <local|remote> <H> <P> <U> <P> - Save Profile\n");
        append_text(
            "  save wg <priv> <pub> <end:port> <ip> - Save WireGuard\n");
        append_text("  settings - Go to settings\n");
        append_text("  exit - Disconnect SSH\n");
        append_text("  clear - Clear terminal\n");
        append_text("  neofetch - Display system info\n");
      } else if (!current_input.empty()) {
        append_text("Unknown command. Type 'help'\n");
      }

      // Save to history
      if (!current_input.empty()) {
        auto it = std::find(command_history.begin(), command_history.end(),
                            current_input);
        if (it != command_history.end()) {
          command_history.erase(it);
        }
        command_history.push_back(current_input);
        history_needs_save = true;
      }
    }

    current_input.clear();
    cursor_pos = 0;
    history_index = -1;
  } else if (key == 8 || key == 127) {
    // Backspace
    if (cursor_pos > 0 && !current_input.empty()) {
      current_input.erase(cursor_pos - 1, 1);
      cursor_pos--;
      if (!ssh_connected) {
        append_text("\b \b");
      }
    }
  } else if (key >= 32 && key <= 126) {
    // Printable character
    current_input.insert(cursor_pos, 1, key);
    cursor_pos++;
    if (!ssh_connected) {
      char buf[2] = {key, 0};
      append_text(buf);
    }
  }

  update_input_display();
}

void SSHTerminal::flush_display_buffer() {
  if (text_buffer.empty())
    return;

  append_text(text_buffer.c_str());
  text_buffer.clear();

  if (bytes_received > 0 && byte_counter_label) {
    char *counter_buf = (char *)malloc(32);
    if (counter_buf) {
      if (bytes_received < 1024) {
        snprintf(counter_buf, 32, "%zu B", bytes_received);
      } else if (bytes_received < 1024 * 1024) {
        snprintf(counter_buf, 32, "%.1f KB", bytes_received / 1024.0);
      } else {
        snprintf(counter_buf, 32, "%.2f MB",
                 bytes_received / (1024.0 * 1024.0));
      }
      lv_async_call(async_update_counter_cb, counter_buf);
    }
  }

  last_display_update = esp_timer_get_time() / 1000;
}

void SSHTerminal::ssh_receive_task(void *param) {
  SSHTerminal *terminal = (SSHTerminal *)param;
  char buffer[4096];

  while (terminal->run_receive_task && terminal->ssh_connected &&
         terminal->channel) {
    int nbytes = ssh_channel_read_nonblocking(terminal->channel, buffer,
                                              sizeof(buffer) - 1, 0);

    if (nbytes > 0) {
      buffer[nbytes] = '\0';
      terminal->process_received_data(buffer, nbytes);
    } else if (nbytes == SSH_ERROR ||
               (terminal->channel && ssh_channel_is_eof(terminal->channel))) {
      break;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }

  terminal->run_receive_task = false;
  terminal->receive_task_handle = nullptr;
}

std::string SSHTerminal::apply_ansi_formatting(const char *data, size_t len) {
  std::string result;
  result.reserve(len * 1.5); // Space for LVGL tags

  for (size_t i = 0; i < len; i++) {
    if (data[i] == '\x1B' || data[i] == '\033') {
      i++;
      if (i >= len)
        break;

      if (data[i] == '[') {
        i++;
        std::string params;
        while (i < len && !((data[i] >= 'A' && data[i] <= 'Z') ||
                            (data[i] >= 'a' && data[i] <= 'z'))) {
          params += data[i];
          i++;
        }

        char cmd = data[i];
        if (cmd == 'm') { // SGR - Select Graphic Rendition (Colors)
          if (params == "0" || params.empty()) {
            result += "#"; // Reset color tag for LVGL
          } else if (params == "31") {
            result += "#FF3333 "; // Red
          } else if (params == "32") {
            result += "#00FF66 "; // Green
          } else if (params == "33") {
            result += "#FFFF00 "; // Yellow
          } else if (params == "34") {
            result += "#3366FF "; // Blue
          } else if (params == "35") {
            result += "#FF33FF "; // Magenta
          } else if (params == "36") {
            result += "#33FFFF "; // Cyan
          }
        }
      } else if (data[i] == ']') {
        // OSC codes - skip
        while (i < len && data[i] != '\007')
          i++;
      }
    } else if (data[i] == '\r') {
      continue;
    } else {
      // Escape special LVGL characters if they appear in raw data
      if (data[i] == '#')
        result += "##";
      else
        result += data[i];
    }
  }

  return result;
}

void SSHTerminal::process_received_data(const char *data, size_t len) {
  bytes_received += len;

  std::string cleaned = apply_ansi_formatting(data, len);
  text_buffer += cleaned;

  int64_t current_time = esp_timer_get_time() / 1000;

  if (text_buffer.size() > 2048) {
    text_buffer = text_buffer.substr(text_buffer.size() - 1024);
  }

  if (current_time - last_display_update >= 100) {
    flush_display_buffer();
  }
}

void SSHTerminal::load_history() {
  preferences.begin("ssh_term", true);
  String history_str = preferences.getString("history", "");
  preferences.end();

  if (history_str.length() > 0) {
    size_t pos = 0;
    std::string hist = history_str.c_str();
    while ((pos = hist.find('\n')) != std::string::npos) {
      std::string cmd = hist.substr(0, pos);
      if (!cmd.empty()) {
        command_history.push_back(cmd);
      }
      hist.erase(0, pos + 1);
    }
    if (!hist.empty()) {
      command_history.push_back(hist);
    }
  }
}

void SSHTerminal::navigate_history(int direction) {
  if (command_history.empty())
    return;

  if (direction > 0) {
    if (history_index < (int)command_history.size() - 1) {
      history_index++;
    }
  } else {
    if (history_index > 0) {
      history_index--;
    } else if (history_index == 0) {
      history_index = -1;
      current_input.clear();
      cursor_pos = 0;
      update_input_display();
      return;
    }
  }

  if (history_index >= 0 && history_index < (int)command_history.size()) {
    int idx = command_history.size() - 1 - history_index;
    current_input = command_history[idx];
    cursor_pos = current_input.length();
    update_input_display();
  }
}

void SSHTerminal::delete_current_history_entry() {
  if (history_index >= 0 && history_index < (int)command_history.size()) {
    int idx = command_history.size() - 1 - history_index;
    command_history.erase(command_history.begin() + idx);
    history_needs_save = true;

    if (history_index >= (int)command_history.size()) {
      history_index = command_history.size() - 1;
    }

    if (history_index >= 0) {
      int new_idx = command_history.size() - 1 - history_index;
      current_input = command_history[new_idx];
      cursor_pos = current_input.length();
    } else {
      current_input.clear();
      cursor_pos = 0;
    }

    update_input_display();
    append_text("[History entry deleted]\n");
  }
}

void SSHTerminal::save_history() {
  if (!history_needs_save)
    return;

  preferences.begin("ssh_term", false);
  std::string hist;
  for (const auto &cmd : command_history) {
    if (!hist.empty())
      hist += '\n';
    hist += cmd;
  }
  preferences.putString("history", hist.c_str());
  preferences.end();

  history_needs_save = false;
}

void SSHTerminal::btn_pulse_anim_cb(void *var, int32_t v) {
  lv_obj_t *obj = (lv_obj_t *)var;
  if (lv_obj_has_state(obj, LV_STATE_FOCUSED)) {
    lv_obj_set_style_shadow_width(obj, v, LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(obj, 100 + (v * 4), LV_STATE_FOCUSED);
  }
}

void SSHTerminal::glitch_anim_cb(void *var, int32_t v) {
  lv_obj_t *obj = (lv_obj_t *)var;
  if (v == 0) {
    lv_obj_set_style_translate_x(obj, 0, 0);
    lv_obj_set_style_transform_scale(obj, 256, 0);
    lv_obj_set_style_transform_angle(obj, 0, 0);
  } else {
    lv_obj_set_style_translate_x(obj, (rand() % 20) - 10, 0); // CRT jitter
    lv_obj_set_style_transform_scale(obj, 256 + (v / 2), 0);
    lv_obj_set_style_transform_angle(obj, (rand() % 40) - 20, 0);
  }
}

void SSHTerminal::trigger_glitch(lv_obj_t *obj) {
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, obj);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)glitch_anim_cb);
  lv_anim_set_values(&a, 50, 0);
  lv_anim_set_time(&a, 150);
  lv_anim_start(&a);
}

void SSHTerminal::title_flicker_cb(void *var, int32_t v) {
  lv_obj_t *obj = (lv_obj_t *)var;
  if (v % 100 > 95) { // Rapid flicker occasional dropout
    lv_obj_set_style_opa(obj, 0, 0);
  } else if (v % 30 < 10) { // Slight dimming
    lv_obj_set_style_opa(obj, 180, 0);
  } else {
    lv_obj_set_style_opa(obj, 255, 0);
  }
}

void SSHTerminal::save_session() {
  if (cumulative_text.empty())
    return;
  preferences.begin("ssh_term", false);
  preferences.putString("session", cumulative_text.c_str());
  preferences.end();
}

void SSHTerminal::load_session() {
  preferences.begin("ssh_term", true);
  String session_str = preferences.getString("session", "");
  preferences.end();
  if (session_str.length() > 0) {
    cumulative_text = session_str.c_str();
  }
}

void SSHTerminal::grid_scroll_anim_cb(void *var, int32_t v) {
  lv_obj_t *obj = (lv_obj_t *)var;
  lv_obj_set_y(obj, v);
  // Fade out as it reaches bottom
  int opa = 12;
  if (v > 280)
    opa = 12 - (v - 280) / 4;
  if (opa < 0)
    opa = 0;
  lv_obj_set_style_bg_opa(obj, opa, 0);
}

void SSHTerminal::display_neofetch_banner() {
    clear_terminal();
    append_text("       #FFD700 .--.        \n");
    append_text("      |o_o |       \n");
    append_text("      |:_/ |       \n");
    append_text("     //   \\ \\      \n");
    append_text("    (|     | )     \n");
    append_text("   /'\\_   _/`\\    \n");
    append_text("   \\___)=(___/    \n\n");
    append_text("#FFFFFF AVERROES TERMINAL#\n");
    append_text("Firmware: v2.7 Elite Ultra\n");
    append_text("MCU: ESP32-S3 @ 240MHz\n");
    append_text("Flash: 16MB\n");
    append_text("PSRAM: 8MB\n");
    append_text("Display: ST7796 480x222\n");
    append_text("--------------------------\n");
}
