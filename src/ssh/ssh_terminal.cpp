/**
 * SSH Terminal for AVERROES Pager
 * Ported from PocketSSH (https://github.com/0015/PocketSSH)
 * Adapted for LilyGo T-LoRa-Pager hardware
 *
 * Theme: Cyberpunk Yellow - Vibrant Yellow on Black with White highlights
 */

#include "ssh_terminal.h"
#include <LilyGoLib.h>
#include <Preferences.h>
#include <esp_timer.h>

static const char *TAG = "SSH_TERMINAL";
static Preferences preferences;
static SSHTerminal *ssht_instance = nullptr;

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

// UI Constants
#define STATUS_BAR_HEIGHT 28
#define INPUT_BAR_HEIGHT 32
#define CORNER_RADIUS 0 // Sharp cyberpunk corners (or 4 for slight rounding)

SSHTerminal::SSHTerminal() {
  ssht_instance = this;
  load_history();
}

SSHTerminal::~SSHTerminal() {
  save_history();
  disconnect();
  wifi_disconnect();
}

lv_obj_t *SSHTerminal::create_terminal_screen() {
  terminal_screen = lv_obj_create(NULL);
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
  lv_obj_set_size(output_container, LV_PCT(100),
                  lv_pct(100) - STATUS_BAR_HEIGHT - INPUT_BAR_HEIGHT - 8);
  lv_obj_set_style_bg_color(output_container, COLOR_BG, 0);
  lv_obj_set_style_border_width(output_container, 0, 0);
  lv_obj_set_style_radius(output_container, 0, 0);
  lv_obj_set_style_pad_all(output_container, 8, 0);
  lv_obj_set_scroll_dir(output_container, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(output_container, LV_SCROLLBAR_MODE_AUTO);

  output_label = lv_label_create(output_container);
  lv_obj_set_width(output_label, LV_PCT(100));
  lv_obj_set_style_text_color(output_label, COLOR_FG, 0);
  lv_obj_set_style_text_font(output_label, &lv_font_montserrat_12, 0);
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

  return terminal_screen;
}

lv_obj_t *SSHTerminal::create_launcher_screen() {
  launcher_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(launcher_screen, COLOR_BG, 0);

  // ═══════════════════════════════════════════════════════════════════════
  // TITLE - Large yellow text
  // ═══════════════════════════════════════════════════════════════════════
  lv_obj_t *title = lv_label_create(launcher_screen);
  lv_obj_set_style_text_color(title, COLOR_FG, 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
  lv_label_set_text(title, "AVERROES SSH");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t *subtitle = lv_label_create(launcher_screen);
  lv_obj_set_style_text_color(subtitle, COLOR_DIM, 0);
  lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_12, 0);
  lv_label_set_text(subtitle, "SELECT CONNECTION TYPE");
  lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 50);

  // ═══════════════════════════════════════════════════════════════════════
  // BUTTONS - Large, navigable
  // ═══════════════════════════════════════════════════════════════════════
  launcher_group = lv_group_create();

  // Button Style
  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, COLOR_BG);
  lv_style_set_border_color(&style_btn, COLOR_DIM);
  lv_style_set_border_width(&style_btn, 2);
  lv_style_set_text_color(&style_btn, COLOR_FG);
  lv_style_set_radius(&style_btn, 4);

  static lv_style_t style_btn_focused;
  lv_style_init(&style_btn_focused);
  lv_style_set_bg_color(&style_btn_focused, COLOR_FG);
  lv_style_set_border_color(&style_btn_focused, COLOR_HIGHLIGHT);
  lv_style_set_text_color(&style_btn_focused, COLOR_BG);

  // LOCAL SSH BUTTON
  lv_obj_t *btn_local = lv_btn_create(launcher_screen);
  lv_obj_set_size(btn_local, 240, 60);
  lv_obj_align(btn_local, LV_ALIGN_CENTER, 0, -20);
  lv_obj_add_style(btn_local, &style_btn, 0);
  lv_obj_add_style(btn_local, &style_btn_focused, LV_STATE_FOCUSED);
  lv_obj_t *label_local = lv_label_create(btn_local);
  lv_label_set_text(label_local, LV_SYMBOL_HOME "  LOCAL SSH");
  lv_obj_center(label_local);
  lv_obj_set_style_text_font(label_local, &lv_font_montserrat_18, 0);
  lv_obj_add_event_cb(btn_local, launcher_event_cb, LV_EVENT_CLICKED,
                      (void *)"local");
  lv_group_add_obj(launcher_group, btn_local);

  // TAILSCALE SSH BUTTON
  lv_obj_t *btn_tail = lv_btn_create(launcher_screen);
  lv_obj_set_size(btn_tail, 240, 60);
  lv_obj_align(btn_tail, LV_ALIGN_CENTER, 0, 50);
  lv_obj_add_style(btn_tail, &style_btn, 0);
  lv_obj_add_style(btn_tail, &style_btn_focused, LV_STATE_FOCUSED);
  lv_obj_t *label_tail = lv_label_create(btn_tail);
  lv_label_set_text(label_tail, LV_SYMBOL_WIFI "  TAILSCALE");
  lv_obj_center(label_tail);
  lv_obj_set_style_text_font(label_tail, &lv_font_montserrat_18, 0);
  lv_obj_add_event_cb(btn_tail, launcher_event_cb, LV_EVENT_CLICKED,
                      (void *)"tailscale");
  lv_group_add_obj(launcher_group, btn_tail);

  return launcher_screen;
}

void SSHTerminal::show_launcher() {
  in_launcher = true;
  lv_scr_load(launcher_screen);
}

void SSHTerminal::show_terminal() {
  in_launcher = false;
  lv_scr_load(terminal_screen);
}

void SSHTerminal::vibrate(uint32_t ms) {
  instance.vibrator();
  // Simplified for now as vibrator() is usually non-blocking short pulse
}

void SSHTerminal::launcher_event_cb(lv_event_t *e) {
  const char *type = (const char *)lv_event_get_user_data(e);
  // This is static, but we need the instance. For now we just print.
  // We'll handle selection in main.cpp or via global pointer.
  Serial.printf("Launcher select: %s\n", type);
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
  if (!preferences.isKey("host")) {
    preferences.end();
    return false;
  }
  profile.host = preferences.getString("host", "").c_str();
  profile.port = preferences.getInt("port", 22);
  profile.user = preferences.getString("user", "").c_str();
  profile.pass = preferences.getString("pass", "").c_str();
  preferences.end();
  return true;
}

void SSHTerminal::connect_to_profile(const char *type) {
  SSHProfile prof;
  if (load_profile(type, prof)) {
    append_text("\nConnecting to [");
    append_text(type);
    append_text("] Profile...\n");
    connect(prof.host.c_str(), prof.port, prof.user.c_str(), prof.pass.c_str());
  } else {
    append_text("\nError: No profile saved for [");
    append_text(type);
    append_text("]\n");
    append_text("Usage: save <local|tailscale> <host> <port> <user> <pass>\n");
  }
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
    update_status_bar();
    return true;
  } else {
    append_text("WiFi connection failed!\n");
    return false;
  }
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

  // Connect
  int rc = ssh_connect(session);
  if (rc != SSH_OK) {
    append_text("SSH connection failed: ");
    append_text(ssh_get_error(session));
    append_text("\n");
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
  xTaskCreate(ssh_receive_task, "ssh_rx", 8192, this, 5, &receive_task_handle);

  return true;
}

void SSHTerminal::disconnect() {
  if (receive_task_handle) {
    vTaskDelete(receive_task_handle);
    receive_task_handle = nullptr;
  }

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
      append_text("> ");
      append_text(current_input.c_str());
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
        // Parse: save <local|tailscale> <host> <port> <user> <pass>
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
              "Usage: save <local|tailscale> <host> <port> <user> <pass>\n");
        }
      } else if (current_input == "home") {
        disconnect();
        show_launcher();
        return; // Skip history for home navigation
      } else if (current_input == "disconnect") {
        wifi_disconnect();
        append_text("WiFi disconnected.\n");
      } else if (current_input == "exit") {
        disconnect();
      } else if (current_input == "clear") {
        clear_terminal();
      } else if (current_input == "help") {
        append_text("Commands:\n");
        append_text("  connect <SSID> <PASS> - Connect WiFi\n");
        append_text("  ssh <HOST> <PORT> <USER> <PASS> - Manual SSH\n");
        append_text("  save <type> <H> <P> <U> <P> - Save Profile\n");
        append_text("  home - Return to Launcher\n");
        append_text("  disconnect - Disconnect WiFi\n");
        append_text("  exit - Disconnect SSH\n");
        append_text("  clear - Clear terminal\n");
        append_text("  help - Show this help\n");
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
    }
  } else if (key >= 32 && key <= 126) {
    // Printable character
    current_input.insert(cursor_pos, 1, key);
    cursor_pos++;
  }

  update_input_display();
}

void SSHTerminal::append_text(const char *text) {
  if (!output_label)
    return;

  const char *current = lv_label_get_text(output_label);
  std::string new_text = current;
  new_text += text;

  // Limit buffer size
  if (new_text.size() > 4096) {
    new_text = new_text.substr(new_text.size() - 2048);
  }

  lv_label_set_text(output_label, new_text.c_str());

  // Scroll to bottom
  lv_obj_t *parent = lv_obj_get_parent(output_label);
  if (parent) {
    lv_obj_scroll_to_y(parent, LV_COORD_MAX, LV_ANIM_OFF);
  }
}

void SSHTerminal::clear_terminal() {
  if (output_label) {
    lv_label_set_text(output_label, "");
  }
  bytes_received = 0;
  if (byte_counter_label) {
    lv_label_set_text(byte_counter_label, "0 B");
  }
}

void SSHTerminal::navigate_history(int direction) {
  if (command_history.empty())
    return;

  if (direction > 0) {
    // Older (up)
    if (history_index < (int)command_history.size() - 1) {
      history_index++;
    }
  } else {
    // Newer (down)
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

void SSHTerminal::update_status_bar() {
  if (!status_bar)
    return;

  std::string status;

  // Status bar is on yellow header, so we use black text + colored symbols
  // concept For visibility, we keep status text black but add colored
  // indicators
  if (!wifi_connected) {
    status = LV_SYMBOL_WIFI " OFF";
    // Red indicator on header (change bg color temporarily? No, just text)
    lv_obj_set_style_text_color(status_bar, COLOR_ERROR, 0);
  } else if (!ssh_connected) {
    status = LV_SYMBOL_WIFI " OK | " LV_SYMBOL_CLOSE " SSH";
    lv_obj_set_style_text_color(status_bar, COLOR_BG, 0); // Black on yellow
  } else {
    status = LV_SYMBOL_WIFI " OK | " LV_SYMBOL_OK " SSH";
    lv_obj_set_style_text_color(status_bar, COLOR_BG, 0); // Black on yellow
  }

  lv_label_set_text(status_bar, status.c_str());
}

void SSHTerminal::update_input_display() {
  if (!input_label)
    return;

  if (cursor_pos > current_input.length()) {
    cursor_pos = current_input.length();
  }

  // Input text only (prompt is separate now)
  std::string display = current_input;

  if (cursor_visible) {
    display.insert(cursor_pos, "|");
  }

  lv_label_set_text(input_label, display.c_str());
}

void SSHTerminal::flush_display_buffer() {
  if (text_buffer.empty())
    return;

  append_text(text_buffer.c_str());
  text_buffer.clear();

  if (bytes_received > 0 && byte_counter_label) {
    char counter_text[32];
    if (bytes_received < 1024) {
      snprintf(counter_text, sizeof(counter_text), "%zu B", bytes_received);
    } else if (bytes_received < 1024 * 1024) {
      snprintf(counter_text, sizeof(counter_text), "%.1f KB",
               bytes_received / 1024.0);
    } else {
      snprintf(counter_text, sizeof(counter_text), "%.2f MB",
               bytes_received / (1024.0 * 1024.0));
    }
    lv_label_set_text(byte_counter_label, counter_text);
  }

  last_display_update = esp_timer_get_time() / 1000;
}

void SSHTerminal::ssh_receive_task(void *param) {
  SSHTerminal *terminal = (SSHTerminal *)param;
  char buffer[4096];

  while (terminal->ssh_connected && terminal->channel) {
    int nbytes = ssh_channel_read_nonblocking(terminal->channel, buffer,
                                              sizeof(buffer) - 1, 0);

    if (nbytes > 0) {
      buffer[nbytes] = '\0';
      terminal->process_received_data(buffer, nbytes);
    } else if (nbytes == SSH_ERROR || ssh_channel_is_eof(terminal->channel)) {
      break;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }

  terminal->disconnect();
  vTaskDelete(NULL);
}

std::string SSHTerminal::strip_ansi_codes(const char *data, size_t len) {
  std::string result;
  result.reserve(len);

  for (size_t i = 0; i < len; i++) {
    if (data[i] == '\x1B' || data[i] == '\033') {
      i++;
      if (i >= len)
        break;

      if (data[i] == '[') {
        i++;
        while (i < len && !((data[i] >= 'A' && data[i] <= 'Z') ||
                            (data[i] >= 'a' && data[i] <= 'z'))) {
          i++;
        }
      } else if (data[i] == ']') {
        i++;
        while (i < len) {
          if (data[i] == '\007')
            break;
          if (data[i] == '\x1B' && i + 1 < len && data[i + 1] == '\\') {
            i++;
            break;
          }
          i++;
        }
      } else if (data[i] == '(' || data[i] == ')') {
        i++;
      }
    } else if (data[i] == '\r') {
      continue;
    } else {
      result += data[i];
    }
  }

  return result;
}

void SSHTerminal::process_received_data(const char *data, size_t len) {
  bytes_received += len;

  std::string cleaned = strip_ansi_codes(data, len);
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
