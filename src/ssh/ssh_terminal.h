/**
 * SSH Terminal for AVERROES Pager
 * Ported from PocketSSH (https://github.com/0015/PocketSSH)
 * Adapted for LilyGo T-LoRa-Pager hardware
 */

#ifndef SSH_TERMINAL_H
#define SSH_TERMINAL_H

#include "UIMessageQueue.h"
#include <Arduino.h>
#include <LilyGoLib.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WireGuard-ESP32.h>
#include <atomic>
#include <libssh/libssh.h>
#include <lvgl.h>
#include <string>
#include <vector>

// Global lock wrappers for cross-file use
extern void lvgl_lock();
extern void lvgl_unlock();
extern void i2c_lock();
extern void i2c_unlock();

// UIMessage from UIMessageQueue.h is used for pooled updates

struct SSHProfile {
  std::string host;
  int port;
  std::string user;
  std::string pass;
  bool active;
};

struct WireGuardConfig {
  std::string private_key;
  std::string remote_public_key;
  std::string endpoint;
  std::string local_ip;
  int port;
  bool active;
};

class SSHTerminal {
public:
  SSHTerminal();
  ~SSHTerminal();

  // Screen creation
  lv_obj_t *create_terminal_screen();
  lv_obj_t *create_launcher_screen();
  void show_launcher();
  void show_terminal();

  // WireGuard management
  bool wg_connect();
  void wg_disconnect();
  void save_wg_config(const char *private_key, const char *public_key,
                      const char *endpoint, const char *local_ip);
  bool load_wg_config(WireGuardConfig &config);

  // WiFi management
  bool wifi_connect(const char *ssid, const char *password);
  bool wifi_auto_connect();
  void wifi_disconnect();
  bool is_wifi_connected() const { return wifi_connected; }

  // SSH connection
  bool connect(const char *host, int port, const char *user, const char *pass);
  void disconnect();
  bool is_ssh_connected() const { return ssh_connected; }

  // Terminal I/O
  void send_command(const char *cmd);
  void handle_key_input(char key);
  void send_special_key(uint8_t key_code);
  void append_text(const char *text);
  void clear_terminal();

  // Profile Management
  void save_profile(const char *type, const char *host, int port,
                    const char *user, const char *pass);
  bool load_profile(const char *type, SSHProfile &profile);
  void connect_to_profile(const char *type);

  // History & Navigation
  void navigate_history(int direction);
  void delete_current_history_entry();

  // Display updates
  void update_status_bar();
  void update_input_display();
  void flush_display_buffer();

  // Hardware feedback
  void vibrate(uint32_t ms = 50);

  // Tasks
  static void ssh_receive_task(void *param);
  static void connection_task(void *param);

  // External access for main.cpp
  lv_group_t *get_launcher_group() { return launcher_group; }
  bool is_in_launcher() const { return in_launcher; }

private:
  // SSH session
  ssh_session session = nullptr;
  ssh_channel channel = nullptr;

  // State
  bool wifi_connected = false;
  bool ssh_connected = false;
  bool in_launcher = true;
  std::atomic<bool> is_connecting = {false};
  std::atomic<bool> run_receive_task = {false};

  // Input handling
  std::string current_input;
  size_t cursor_pos = 0;
  bool cursor_visible = true;

  // Command history
  std::vector<std::string> command_history;
  int history_index = -1;
  bool history_needs_save = false;

  // Display buffer
  std::string cumulative_text;
  std::string text_buffer;
  size_t bytes_received = 0;
  int64_t last_display_update = 0;

  // LVGL objects
  lv_obj_t *terminal_screen = nullptr;
  lv_obj_t *launcher_screen = nullptr;
  lv_obj_t *output_label = nullptr;
  lv_obj_t *input_label = nullptr;
  lv_obj_t *status_bar = nullptr;
  lv_obj_t *byte_counter_label = nullptr;

  // Groups
  lv_group_t *launcher_group = nullptr;

  // Task handles
  TaskHandle_t receive_task_handle = nullptr;
  TaskHandle_t connection_task_handle = nullptr;
  WireGuard wg;
  UIMessageQueue ui_queue;

  static SSHTerminal *ssht_instance;
  // Helper methods
  std::string apply_ansi_formatting(const char *data, size_t len);
  void process_received_data(const char *data, size_t len);
  void load_history();
  void save_history();
  static void launcher_event_cb(lv_event_t *e);
  static void btn_pulse_anim_cb(void *var, int32_t v);
  static void glitch_anim_cb(void *var, int32_t v);
  static void grid_scroll_anim_cb(void *var, int32_t v);
  static void title_flicker_cb(void *var, int32_t v);

  // Async UI handlers
  static void async_append_text_cb(void *param);
  static void async_update_status_cb(void *param);
  static void async_show_terminal_cb(void *param);
  static void async_show_launcher_cb(void *param);
  static void async_update_input_cb(void *param);
  static void async_update_counter_cb(void *param);
  void trigger_glitch(lv_obj_t *obj);

  void save_session();
  void load_session();
};

#endif // SSH_TERMINAL_H
