/**
 * SSH Terminal for AVERROES Pager
 * Ported from PocketSSH (https://github.com/0015/PocketSSH)
 * Adapted for LilyGo T-LoRa-Pager hardware
 */

#ifndef SSH_TERMINAL_H
#define SSH_TERMINAL_H

#include <Arduino.h>
#include <WiFi.h>
#include <libssh/libssh.h>
#include <lvgl.h>
#include <string>
#include <vector>

class SSHTerminal {
public:
  SSHTerminal();
  ~SSHTerminal();

  // Screen creation
  lv_obj_t *create_terminal_screen();

  // WiFi management
  bool wifi_connect(const char *ssid, const char *password);
  void wifi_disconnect();
  bool is_wifi_connected() const { return wifi_connected; }

  // SSH connection
  bool connect(const char *host, int port, const char *user, const char *pass);
  void disconnect();
  bool is_ssh_connected() const { return ssh_connected; }

  // Terminal I/O
  void send_command(const char *cmd);
  void handle_key_input(char key);
  void send_special_key(uint8_t key_code); // Ctrl+C, Tab, Esc, etc.
  void append_text(const char *text);
  void clear_terminal();

  // History navigation
  void navigate_history(int direction);
  void delete_current_history_entry();

  // Display updates
  void update_status_bar();
  void update_input_display();
  void flush_display_buffer();

  // Tasks
  static void ssh_receive_task(void *param);

private:
  // SSH session
  ssh_session session = nullptr;
  ssh_channel channel = nullptr;

  // State
  bool wifi_connected = false;
  bool ssh_connected = false;

  // Input handling
  std::string current_input;
  size_t cursor_pos = 0;
  bool cursor_visible = true;

  // Command history
  std::vector<std::string> command_history;
  int history_index = -1;
  bool history_needs_save = false;

  // Display buffer
  std::string text_buffer;
  size_t bytes_received = 0;
  int64_t last_display_update = 0;

  // LVGL objects
  lv_obj_t *terminal_screen = nullptr;
  lv_obj_t *output_label = nullptr;
  lv_obj_t *input_label = nullptr;
  lv_obj_t *status_bar = nullptr;
  lv_obj_t *byte_counter_label = nullptr;

  // Task handle
  TaskHandle_t receive_task_handle = nullptr;

  // Helper methods
  std::string strip_ansi_codes(const char *data, size_t len);
  void process_received_data(const char *data, size_t len);
  void load_history();
  void save_history();
};

#endif // SSH_TERMINAL_H
