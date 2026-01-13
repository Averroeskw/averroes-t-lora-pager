/**
 * ═══════════════════════════════════════════════════════════════════════════
 *  █████╗ ██╗   ██╗███████╗██████╗ ██████╗  ██████╗ ███████╗███████╗
 * ██╔══██╗██║   ██║██╔════╝██╔══██╗██╔══██╗██╔═══██╗██╔════╝██╔════╝
 * ███████║██║   ██║█████╗  ██████╔╝██████╔╝██║   ██║█████╗  ███████╗
 * ██╔══██║╚██╗ ██╔╝██╔══╝  ██╔══██╗██╔══██╗██║   ██║██╔══╝  ╚════██║
 * ██║  ██║ ╚████╔╝ ███████╗██║  ██║██║  ██║╚██████╔╝███████╗███████║
 * ╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚══════╝
 * ═══════════════════════════════════════════════════════════════════════════
 * AVERROES T-LoRa Pager - Enhanced UI System
 * Cyberpunk Theme with Orange/Amber Accent
 * Resolution: 480x222 (Horizontal)
 * ═══════════════════════════════════════════════════════════════════════════
 */

#ifndef __AVERROES_UI_H__
#define __AVERROES_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/*═══════════════════════════════════════════════════════════════════════════
 * DISPLAY CONFIGURATION
 *═══════════════════════════════════════════════════════════════════════════*/
#define UI_DISP_WIDTH   480
#define UI_DISP_HEIGHT  222

/*═══════════════════════════════════════════════════════════════════════════
 * CYBERPUNK COLOR PALETTE - AVERROES THEME
 *═══════════════════════════════════════════════════════════════════════════*/

/* Primary Colors */
#define UI_COLOR_PRIMARY        lv_color_hex(0xFF6B00)   /* AVERROES Orange */
#define UI_COLOR_PRIMARY_DARK   lv_color_hex(0xCC5500)   /* Dark Orange */
#define UI_COLOR_PRIMARY_LIGHT  lv_color_hex(0xFF8833)   /* Light Orange */

/* Secondary & Accent */
#define UI_COLOR_SECONDARY      lv_color_hex(0x00FFAA)   /* Cyan/Teal */
#define UI_COLOR_ACCENT         lv_color_hex(0xFF00AA)   /* Magenta */
#define UI_COLOR_ACCENT_ALT     lv_color_hex(0xFFAA00)   /* Amber */

/* Background Layers */
#define UI_COLOR_BG_DARK        lv_color_hex(0x050505)   /* Deepest Black */
#define UI_COLOR_BG_BASE        lv_color_hex(0x0D0D0D)   /* Base Background */
#define UI_COLOR_BG_SURFACE     lv_color_hex(0x1A1A1A)   /* Surface */
#define UI_COLOR_BG_ELEVATED    lv_color_hex(0x2D2D2D)   /* Elevated Surface */
#define UI_COLOR_BG_HIGHLIGHT   lv_color_hex(0x3D3D3D)   /* Highlight */

/* Text Colors */
#define UI_COLOR_TEXT_PRIMARY   lv_color_hex(0xFF6B00)   /* Orange Text */
#define UI_COLOR_TEXT_SECONDARY lv_color_hex(0x888888)   /* Dimmed Text */
#define UI_COLOR_TEXT_TERTIARY  lv_color_hex(0x555555)   /* Very Dim */
#define UI_COLOR_TEXT_BRIGHT    lv_color_hex(0xFFFFFF)   /* White */

/* Status Colors */
#define UI_COLOR_SUCCESS        lv_color_hex(0x00FF66)   /* Green */
#define UI_COLOR_WARNING        lv_color_hex(0xFFAA00)   /* Amber */
#define UI_COLOR_ERROR          lv_color_hex(0xFF0044)   /* Red */
#define UI_COLOR_INFO           lv_color_hex(0x00AAFF)   /* Blue */

/* Border & Lines */
#define UI_COLOR_BORDER         lv_color_hex(0x333333)   /* Default Border */
#define UI_COLOR_BORDER_FOCUS   lv_color_hex(0xFF6B00)   /* Focused Border */
#define UI_COLOR_DIVIDER        lv_color_hex(0x222222)   /* Divider Lines */

/* Glow Effects (for shadows) */
#define UI_COLOR_GLOW_ORANGE    lv_color_hex(0xFF6B00)
#define UI_COLOR_GLOW_CYAN      lv_color_hex(0x00FFAA)

/*═══════════════════════════════════════════════════════════════════════════
 * THEME DEFINITIONS
 *═══════════════════════════════════════════════════════════════════════════*/
typedef enum {
    THEME_CYBERPUNK_ORANGE = 0,  /* Default AVERROES Theme */
    THEME_CYBERPUNK_CYAN,        /* Cyan/Teal variant */
    THEME_CYBERPUNK_MAGENTA,     /* Magenta/Pink variant */
    THEME_MATRIX_GREEN,          /* Classic Matrix green */
    THEME_AMBER_RETRO,           /* Amber retro terminal */
    THEME_COUNT
} ui_theme_t;

typedef struct {
    const char* name;
    uint32_t primary;      /* Hex color value - use lv_color_hex() to convert */
    uint32_t secondary;
    uint32_t text;
    uint32_t background;
    uint32_t surface;
} ui_theme_colors_t;

/*═══════════════════════════════════════════════════════════════════════════
 * SPACING & DIMENSIONS
 *═══════════════════════════════════════════════════════════════════════════*/
#define UI_SPACING_XS       4
#define UI_SPACING_SM       8
#define UI_SPACING_MD       12
#define UI_SPACING_LG       16
#define UI_SPACING_XL       24
#define UI_SPACING_XXL      32

#define UI_RADIUS_SM        4
#define UI_RADIUS_MD        8
#define UI_RADIUS_LG        12
#define UI_RADIUS_ROUND     999

#define UI_STATUSBAR_H      28
#define UI_NAVBAR_H         48
#define UI_CONTENT_PAD      8

/*═══════════════════════════════════════════════════════════════════════════
 * FONT DECLARATIONS
 * Custom fonts - uncomment when font files are added to project
 *═══════════════════════════════════════════════════════════════════════════*/
/* Future custom fonts - currently using built-in lv_font_montserrat_* */
/* LV_FONT_DECLARE(ui_font_mono_12); */
/* LV_FONT_DECLARE(ui_font_mono_14); */
/* LV_FONT_DECLARE(ui_font_mono_16); */
/* LV_FONT_DECLARE(ui_font_bold_18); */
/* LV_FONT_DECLARE(ui_font_bold_20); */
/* LV_FONT_DECLARE(ui_font_icons_16); */

/*═══════════════════════════════════════════════════════════════════════════
 * SCREEN DECLARATIONS
 *═══════════════════════════════════════════════════════════════════════════*/
extern lv_obj_t* ui_screen_home;
extern lv_obj_t* ui_screen_connectivity;
extern lv_obj_t* ui_screen_display;
extern lv_obj_t* ui_screen_system;
extern lv_obj_t* ui_screen_info;

/*═══════════════════════════════════════════════════════════════════════════
 * PAGE ENUMERATION
 *═══════════════════════════════════════════════════════════════════════════*/
typedef enum {
    PAGE_HOME = 0,
    PAGE_CONNECTIVITY,
    PAGE_DISPLAY,
    PAGE_SYSTEM,
    PAGE_INFO,
    PAGE_COUNT
} ui_page_t;

/*═══════════════════════════════════════════════════════════════════════════
 * UI STATE
 *═══════════════════════════════════════════════════════════════════════════*/
typedef struct {
    ui_page_t currentPage;
    ui_theme_t currentTheme;
    uint8_t brightness;
    bool soundEnabled;
    bool hapticEnabled;
    uint8_t volume;
    uint8_t hapticIntensity;
} ui_state_t;

extern ui_state_t ui_state;

/*═══════════════════════════════════════════════════════════════════════════
 * UI INITIALIZATION & NAVIGATION
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_init(void);
void ui_load_page(ui_page_t page);
void ui_load_page_anim(ui_page_t page, lv_scr_load_anim_t anim);
void ui_set_theme(ui_theme_t theme);
void ui_apply_theme(void);
void ui_update_status_bar(void);

/*═══════════════════════════════════════════════════════════════════════════
 * PAGE SETUP FUNCTIONS
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_setup_home_page(void);
void ui_setup_connectivity_page(void);
void ui_setup_display_page(void);
void ui_setup_system_page(void);
void ui_setup_info_page(void);

/*═══════════════════════════════════════════════════════════════════════════
 * COMPONENT BUILDERS
 *═══════════════════════════════════════════════════════════════════════════*/
lv_obj_t* ui_create_status_bar(lv_obj_t* parent);
lv_obj_t* ui_create_nav_bar(lv_obj_t* parent);
lv_obj_t* ui_create_terminal_area(lv_obj_t* parent);
lv_obj_t* ui_create_settings_row(lv_obj_t* parent, const char* icon, const char* label);
lv_obj_t* ui_create_card(lv_obj_t* parent, const char* title);
lv_obj_t* ui_create_icon_button(lv_obj_t* parent, const char* icon, const char* label);
lv_obj_t* ui_create_slider_row(lv_obj_t* parent, const char* label, int32_t min, int32_t max, int32_t val);
lv_obj_t* ui_create_switch_row(lv_obj_t* parent, const char* label, bool state);
lv_obj_t* ui_create_sensor_gauge(lv_obj_t* parent, const char* label, int32_t min, int32_t max);
lv_obj_t* ui_create_progress_bar(lv_obj_t* parent, const char* label);

/*═══════════════════════════════════════════════════════════════════════════
 * INPUT HANDLING
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_handle_encoder_rotate(int8_t diff);
void ui_handle_encoder_click(void);
void ui_handle_encoder_long_press(void);
void ui_handle_key_press(uint8_t key);

/*═══════════════════════════════════════════════════════════════════════════
 * UPDATE FUNCTIONS
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_update_wifi_status(bool connected, int8_t rssi);
void ui_update_lora_status(bool active, int8_t rssi);
void ui_update_gps_status(bool fix, uint8_t satellites);
void ui_update_battery(uint8_t percent, bool charging);
void ui_update_terminal(const char* text);
void ui_append_terminal(const char* text);
void ui_clear_terminal(void);

/*═══════════════════════════════════════════════════════════════════════════
 * ANIMATION HELPERS
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_anim_fade_in(lv_obj_t* obj, uint32_t delay);
void ui_anim_slide_in(lv_obj_t* obj, lv_dir_t dir, uint32_t delay);
void ui_anim_pulse(lv_obj_t* obj);
void ui_anim_glow(lv_obj_t* obj, lv_color_t color);

/*═══════════════════════════════════════════════════════════════════════════
 * ICON DEFINITIONS (Unicode symbols for built-in LVGL symbol font)
 *═══════════════════════════════════════════════════════════════════════════*/
#define UI_ICON_WIFI        LV_SYMBOL_WIFI
#define UI_ICON_BLUETOOTH   LV_SYMBOL_BLUETOOTH
#define UI_ICON_GPS         LV_SYMBOL_GPS
#define UI_ICON_BATTERY     LV_SYMBOL_BATTERY_FULL
#define UI_ICON_SETTINGS    LV_SYMBOL_SETTINGS
#define UI_ICON_HOME        LV_SYMBOL_HOME
#define UI_ICON_REFRESH     LV_SYMBOL_REFRESH
#define UI_ICON_POWER       LV_SYMBOL_POWER
#define UI_ICON_AUDIO       LV_SYMBOL_AUDIO
#define UI_ICON_VOLUME      LV_SYMBOL_VOLUME_MAX
#define UI_ICON_KEYBOARD    LV_SYMBOL_KEYBOARD
#define UI_ICON_USB         LV_SYMBOL_USB
#define UI_ICON_SD          LV_SYMBOL_SD_CARD
#define UI_ICON_FOLDER      LV_SYMBOL_DIRECTORY
#define UI_ICON_FILE        LV_SYMBOL_FILE
#define UI_ICON_EDIT        LV_SYMBOL_EDIT
#define UI_ICON_SAVE        LV_SYMBOL_SAVE
#define UI_ICON_UPLOAD      LV_SYMBOL_UPLOAD
#define UI_ICON_DOWNLOAD    LV_SYMBOL_DOWNLOAD
#define UI_ICON_TRASH       LV_SYMBOL_TRASH
#define UI_ICON_BELL        LV_SYMBOL_BELL
#define UI_ICON_PLAY        LV_SYMBOL_PLAY
#define UI_ICON_PAUSE       LV_SYMBOL_PAUSE
#define UI_ICON_STOP        LV_SYMBOL_STOP
#define UI_ICON_PREV        LV_SYMBOL_PREV
#define UI_ICON_NEXT        LV_SYMBOL_NEXT
#define UI_ICON_SHUFFLE     LV_SYMBOL_SHUFFLE
#define UI_ICON_LOOP        LV_SYMBOL_LOOP
#define UI_ICON_LEFT        LV_SYMBOL_LEFT
#define UI_ICON_RIGHT       LV_SYMBOL_RIGHT
#define UI_ICON_UP          LV_SYMBOL_UP
#define UI_ICON_DOWN        LV_SYMBOL_DOWN
#define UI_ICON_OK          LV_SYMBOL_OK
#define UI_ICON_CLOSE       LV_SYMBOL_CLOSE
#define UI_ICON_PLUS        LV_SYMBOL_PLUS
#define UI_ICON_MINUS       LV_SYMBOL_MINUS
#define UI_ICON_CHARGE      LV_SYMBOL_CHARGE
#define UI_ICON_WARNING     LV_SYMBOL_WARNING
#define UI_ICON_NEW_LINE    LV_SYMBOL_NEW_LINE

#ifdef __cplusplus
}
#endif

#endif /* __AVERROES_UI_H__ */
