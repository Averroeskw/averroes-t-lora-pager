/**
 * ═══════════════════════════════════════════════════════════════════════════
 * AVERROES T-LoRa Pager - UI Implementation
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include "ui.h"
#include <string.h>
#include <stdio.h>

/*═══════════════════════════════════════════════════════════════════════════
 * GLOBAL STATE & SCREENS
 *═══════════════════════════════════════════════════════════════════════════*/
lv_obj_t* ui_screen_home = NULL;
lv_obj_t* ui_screen_connectivity = NULL;
lv_obj_t* ui_screen_display = NULL;
lv_obj_t* ui_screen_system = NULL;
lv_obj_t* ui_screen_info = NULL;

ui_state_t ui_state = {
    .currentPage = PAGE_HOME,
    .currentTheme = THEME_CYBERPUNK_ORANGE,
    .brightness = 200,
    .soundEnabled = true,
    .hapticEnabled = true,
    .volume = 70,
    .hapticIntensity = 50
};

/* Theme color definitions (hex values, use lv_color_hex() to convert) */
static const ui_theme_colors_t theme_colors[THEME_COUNT] = {
    {"Cyberpunk Orange", 0xFF6B00, 0x00FFAA, 0xFF6B00, 0x0D0D0D, 0x1A1A1A},
    {"Cyberpunk Cyan",   0x00FFAA, 0xFF6B00, 0x00FFAA, 0x0D0D0D, 0x1A1A1A},
    {"Cyberpunk Magenta",0xFF00AA, 0x00FFAA, 0xFF00AA, 0x0D0D0D, 0x1A1A1A},
    {"Matrix Green",     0x00FF00, 0x008800, 0x00FF00, 0x000000, 0x0A0A0A},
    {"Amber Retro",      0xFFAA00, 0x885500, 0xFFAA00, 0x0A0500, 0x1A1000}
};

/* Internal references */
static lv_obj_t* status_bar = NULL;
static lv_obj_t* terminal_ta = NULL;
static lv_group_t* input_group = NULL;

/* Status bar widgets */
static lv_obj_t* lbl_wifi_icon = NULL;
static lv_obj_t* lbl_lora_icon = NULL;
static lv_obj_t* lbl_gps_icon = NULL;
static lv_obj_t* lbl_battery_icon = NULL;
static lv_obj_t* lbl_time = NULL;

/*═══════════════════════════════════════════════════════════════════════════
 * STYLE DEFINITIONS
 *═══════════════════════════════════════════════════════════════════════════*/
static lv_style_t style_screen;
static lv_style_t style_statusbar;
static lv_style_t style_card;
static lv_style_t style_card_title;
static lv_style_t style_btn;
static lv_style_t style_btn_pressed;
static lv_style_t style_btn_focused;
static lv_style_t style_terminal;
static lv_style_t style_list_item;
static lv_style_t style_slider;
static lv_style_t style_switch;
static lv_style_t style_nav_btn;
static lv_style_t style_nav_btn_active;

static void init_styles(void) {
    /* Screen base style */
    lv_style_init(&style_screen);
    lv_style_set_bg_color(&style_screen, UI_COLOR_BG_BASE);
    lv_style_set_bg_opa(&style_screen, LV_OPA_COVER);
    lv_style_set_text_color(&style_screen, UI_COLOR_TEXT_PRIMARY);
    lv_style_set_text_font(&style_screen, &lv_font_montserrat_14);
    
    /* Status bar style */
    lv_style_init(&style_statusbar);
    lv_style_set_bg_color(&style_statusbar, UI_COLOR_BG_DARK);
    lv_style_set_bg_opa(&style_statusbar, LV_OPA_COVER);
    lv_style_set_border_width(&style_statusbar, 0);
    lv_style_set_border_side(&style_statusbar, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&style_statusbar, UI_COLOR_PRIMARY);
    lv_style_set_pad_all(&style_statusbar, 4);
    
    /* Card style */
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, UI_COLOR_BG_SURFACE);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, UI_RADIUS_MD);
    lv_style_set_border_width(&style_card, 1);
    lv_style_set_border_color(&style_card, UI_COLOR_BORDER);
    lv_style_set_pad_all(&style_card, UI_SPACING_SM);
    lv_style_set_shadow_width(&style_card, 8);
    lv_style_set_shadow_color(&style_card, lv_color_black());
    lv_style_set_shadow_opa(&style_card, LV_OPA_30);
    
    /* Card title style */
    lv_style_init(&style_card_title);
    lv_style_set_text_color(&style_card_title, UI_COLOR_PRIMARY);
    lv_style_set_text_font(&style_card_title, &lv_font_montserrat_16);
    
    /* Button style */
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, UI_COLOR_BG_ELEVATED);
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_radius(&style_btn, UI_RADIUS_SM);
    lv_style_set_border_width(&style_btn, 1);
    lv_style_set_border_color(&style_btn, UI_COLOR_BORDER);
    lv_style_set_text_color(&style_btn, UI_COLOR_TEXT_PRIMARY);
    lv_style_set_pad_all(&style_btn, UI_SPACING_SM);
    
    /* Button pressed style */
    lv_style_init(&style_btn_pressed);
    lv_style_set_bg_color(&style_btn_pressed, UI_COLOR_PRIMARY_DARK);
    lv_style_set_border_color(&style_btn_pressed, UI_COLOR_PRIMARY);
    
    /* Button focused style */
    lv_style_init(&style_btn_focused);
    lv_style_set_border_width(&style_btn_focused, 2);
    lv_style_set_border_color(&style_btn_focused, UI_COLOR_PRIMARY);
    lv_style_set_shadow_width(&style_btn_focused, 12);
    lv_style_set_shadow_color(&style_btn_focused, UI_COLOR_GLOW_ORANGE);
    lv_style_set_shadow_opa(&style_btn_focused, LV_OPA_50);
    
    /* Terminal style */
    lv_style_init(&style_terminal);
    lv_style_set_bg_color(&style_terminal, UI_COLOR_BG_DARK);
    lv_style_set_bg_opa(&style_terminal, LV_OPA_COVER);
    lv_style_set_text_color(&style_terminal, UI_COLOR_PRIMARY);
    lv_style_set_text_font(&style_terminal, &lv_font_montserrat_10);
    lv_style_set_border_width(&style_terminal, 1);
    lv_style_set_border_color(&style_terminal, UI_COLOR_BORDER);
    lv_style_set_radius(&style_terminal, UI_RADIUS_SM);
    lv_style_set_pad_all(&style_terminal, UI_SPACING_XS);
    
    /* List item style */
    lv_style_init(&style_list_item);
    lv_style_set_bg_color(&style_list_item, UI_COLOR_BG_SURFACE);
    lv_style_set_bg_opa(&style_list_item, LV_OPA_COVER);
    lv_style_set_border_width(&style_list_item, 0);
    lv_style_set_border_side(&style_list_item, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&style_list_item, UI_COLOR_DIVIDER);
    lv_style_set_pad_ver(&style_list_item, UI_SPACING_SM);
    lv_style_set_pad_hor(&style_list_item, UI_SPACING_SM);
    
    /* Slider style */
    lv_style_init(&style_slider);
    lv_style_set_bg_color(&style_slider, UI_COLOR_BG_ELEVATED);
    lv_style_set_bg_opa(&style_slider, LV_OPA_COVER);
    lv_style_set_radius(&style_slider, UI_RADIUS_ROUND);
    
    /* Switch style */
    lv_style_init(&style_switch);
    lv_style_set_bg_color(&style_switch, UI_COLOR_BG_ELEVATED);
    lv_style_set_radius(&style_switch, UI_RADIUS_ROUND);
    
    /* Nav button style */
    lv_style_init(&style_nav_btn);
    lv_style_set_bg_color(&style_nav_btn, lv_color_make(0, 0, 0));
    lv_style_set_bg_opa(&style_nav_btn, LV_OPA_TRANSP);
    lv_style_set_text_color(&style_nav_btn, UI_COLOR_TEXT_SECONDARY);
    lv_style_set_pad_all(&style_nav_btn, UI_SPACING_XS);
    
    /* Nav button active style */
    lv_style_init(&style_nav_btn_active);
    lv_style_set_text_color(&style_nav_btn_active, UI_COLOR_PRIMARY);
}

/*═══════════════════════════════════════════════════════════════════════════
 * UI INITIALIZATION
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_init(void) {
    /* Initialize styles */
    init_styles();
    
    /* Create input group for encoder navigation */
    input_group = lv_group_create();
    lv_group_set_default(input_group);
    
    /* Create all screens */
    ui_setup_home_page();
    ui_setup_connectivity_page();
    ui_setup_display_page();
    ui_setup_system_page();
    ui_setup_info_page();
    
    /* Load home page */
    ui_load_page(PAGE_HOME);
}

/*═══════════════════════════════════════════════════════════════════════════
 * PAGE NAVIGATION
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_load_page(ui_page_t page) {
    ui_load_page_anim(page, LV_SCR_LOAD_ANIM_FADE_IN);
}

void ui_load_page_anim(ui_page_t page, lv_scr_load_anim_t anim) {
    lv_obj_t* target = NULL;
    
    switch(page) {
        case PAGE_HOME:        target = ui_screen_home; break;
        case PAGE_CONNECTIVITY: target = ui_screen_connectivity; break;
        case PAGE_DISPLAY:     target = ui_screen_display; break;
        case PAGE_SYSTEM:      target = ui_screen_system; break;
        case PAGE_INFO:        target = ui_screen_info; break;
        default: return;
    }
    
    if(target) {
        ui_state.currentPage = page;
        lv_scr_load_anim(target, anim, 200, 0, false);
    }
}

/*═══════════════════════════════════════════════════════════════════════════
 * STATUS BAR COMPONENT
 *═══════════════════════════════════════════════════════════════════════════*/
lv_obj_t* ui_create_status_bar(lv_obj_t* parent) {
    lv_obj_t* bar = lv_obj_create(parent);
    lv_obj_add_style(bar, &style_statusbar, 0);
    lv_obj_set_size(bar, UI_DISP_WIDTH, UI_STATUSBAR_H);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    
    /* Left section: Status icons */
    lbl_wifi_icon = lv_label_create(bar);
    lv_label_set_text(lbl_wifi_icon, UI_ICON_WIFI);
    lv_obj_set_style_text_color(lbl_wifi_icon, UI_COLOR_TEXT_SECONDARY, 0);
    lv_obj_align(lbl_wifi_icon, LV_ALIGN_LEFT_MID, 0, 0);
    
    lbl_lora_icon = lv_label_create(bar);
    lv_label_set_text(lbl_lora_icon, LV_SYMBOL_WIFI);  /* LoRa */
    lv_obj_set_style_text_color(lbl_lora_icon, UI_COLOR_TEXT_SECONDARY, 0);
    lv_obj_align_to(lbl_lora_icon, lbl_wifi_icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
    
    lbl_gps_icon = lv_label_create(bar);
    lv_label_set_text(lbl_gps_icon, UI_ICON_GPS);
    lv_obj_set_style_text_color(lbl_gps_icon, UI_COLOR_TEXT_SECONDARY, 0);
    lv_obj_align_to(lbl_gps_icon, lbl_lora_icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
    
    /* Center: Time */
    lbl_time = lv_label_create(bar);
    lv_label_set_text(lbl_time, "12:00");
    lv_obj_set_style_text_color(lbl_time, UI_COLOR_TEXT_PRIMARY, 0);
    lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, 0);
    
    /* Right: Battery */
    lbl_battery_icon = lv_label_create(bar);
    lv_label_set_text(lbl_battery_icon, UI_ICON_BATTERY " 100%");
    lv_obj_set_style_text_color(lbl_battery_icon, UI_COLOR_SUCCESS, 0);
    lv_obj_align(lbl_battery_icon, LV_ALIGN_RIGHT_MID, -4, 0);
    
    status_bar = bar;
    return bar;
}

/*═══════════════════════════════════════════════════════════════════════════
 * NAV BAR COMPONENT
 *═══════════════════════════════════════════════════════════════════════════*/
static void nav_btn_event_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    ui_page_t page = (ui_page_t)(intptr_t)lv_obj_get_user_data(btn);
    ui_load_page_anim(page, LV_SCR_LOAD_ANIM_FADE_IN);
}

lv_obj_t* ui_create_nav_bar(lv_obj_t* parent) {
    lv_obj_t* nav = lv_obj_create(parent);
    lv_obj_set_size(nav, UI_DISP_WIDTH, UI_NAVBAR_H);
    lv_obj_align(nav, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(nav, UI_COLOR_BG_DARK, 0);
    lv_obj_set_style_border_width(nav, 1, 0);
    lv_obj_set_style_border_side(nav, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_color(nav, UI_COLOR_BORDER, 0);
    lv_obj_set_style_pad_all(nav, 4, 0);
    lv_obj_clear_flag(nav, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(nav, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(nav, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    const char* icons[] = {LV_SYMBOL_HOME, LV_SYMBOL_WIFI, LV_SYMBOL_IMAGE, LV_SYMBOL_SETTINGS, LV_SYMBOL_LIST};
    const char* labels[] = {"Home", "Net", "Theme", "Sys", "Info"};
    
    for(int i = 0; i < 5; i++) {
        lv_obj_t* btn = lv_btn_create(nav);
        lv_obj_add_style(btn, &style_nav_btn, 0);
        lv_obj_set_size(btn, 38, 38);
        lv_obj_set_user_data(btn, (void*)(intptr_t)i);
        lv_obj_add_event_cb(btn, nav_btn_event_cb, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(input_group, btn);
        
        lv_obj_t* icon = lv_label_create(btn);
        lv_label_set_text(icon, icons[i]);
        lv_obj_center(icon);
        
        if(i == ui_state.currentPage) {
            lv_obj_add_style(btn, &style_nav_btn_active, 0);
        }
    }
    
    return nav;
}

/*═══════════════════════════════════════════════════════════════════════════
 * CARD COMPONENT
 *═══════════════════════════════════════════════════════════════════════════*/
lv_obj_t* ui_create_card(lv_obj_t* parent, const char* title) {
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_add_style(card, &style_card, 0);
    lv_obj_set_width(card, UI_DISP_WIDTH - UI_SPACING_MD * 2);
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    
    if(title && strlen(title) > 0) {
        lv_obj_t* lbl_title = lv_label_create(card);
        lv_label_set_text(lbl_title, title);
        lv_obj_add_style(lbl_title, &style_card_title, 0);
        lv_obj_set_width(lbl_title, LV_PCT(100));
        
        /* Divider line */
        lv_obj_t* line = lv_obj_create(card);
        lv_obj_set_size(line, LV_PCT(100), 1);
        lv_obj_set_style_bg_color(line, UI_COLOR_BORDER, 0);
        lv_obj_set_style_radius(line, 0, 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_set_style_pad_all(line, 0, 0);
    }
    
    return card;
}

/*═══════════════════════════════════════════════════════════════════════════
 * SETTINGS ROW COMPONENTS
 *═══════════════════════════════════════════════════════════════════════════*/
lv_obj_t* ui_create_slider_row(lv_obj_t* parent, const char* label, int32_t min, int32_t max, int32_t val) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, UI_SPACING_XS, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* lbl = lv_label_create(row);
    lv_label_set_text(lbl, label);
    lv_obj_set_style_text_color(lbl, UI_COLOR_TEXT_SECONDARY, 0);
    
    lv_obj_t* slider = lv_slider_create(row);
    lv_obj_set_width(slider, LV_PCT(100));
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, val, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, UI_COLOR_BG_ELEVATED, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, UI_COLOR_PRIMARY, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, UI_COLOR_PRIMARY, LV_PART_KNOB);
    lv_group_add_obj(input_group, slider);
    
    return row;
}

lv_obj_t* ui_create_switch_row(lv_obj_t* parent, const char* label, bool state) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, UI_SPACING_XS, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* lbl = lv_label_create(row);
    lv_label_set_text(lbl, label);
    lv_obj_set_style_text_color(lbl, UI_COLOR_TEXT_PRIMARY, 0);
    
    lv_obj_t* sw = lv_switch_create(row);
    if(state) lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw, UI_COLOR_BG_ELEVATED, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw, UI_COLOR_PRIMARY, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_group_add_obj(input_group, sw);
    
    return row;
}

lv_obj_t* ui_create_icon_button(lv_obj_t* parent, const char* icon, const char* label) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_add_style(btn, &style_btn, 0);
    lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &style_btn_focused, LV_STATE_FOCUSED);
    lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(btn, UI_SPACING_SM, 0);
    lv_group_add_obj(input_group, btn);
    
    if(icon) {
        lv_obj_t* ico = lv_label_create(btn);
        lv_label_set_text(ico, icon);
        lv_obj_set_style_text_color(ico, UI_COLOR_PRIMARY, 0);
    }
    
    if(label) {
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, label);
        lv_obj_set_style_text_color(lbl, UI_COLOR_TEXT_PRIMARY, 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
    }
    
    return btn;
}

/*═══════════════════════════════════════════════════════════════════════════
 * PAGE 1: HOME / TERMINAL + DEVICE INFO
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_setup_home_page(void) {
    ui_screen_home = lv_obj_create(NULL);
    lv_obj_add_style(ui_screen_home, &style_screen, 0);
    lv_obj_clear_flag(ui_screen_home, LV_OBJ_FLAG_SCROLLABLE);

    /* Status bar */
    ui_create_status_bar(ui_screen_home);

    /* Main content area - two columns */
    lv_obj_t* content = lv_obj_create(ui_screen_home);
    lv_obj_set_size(content, UI_DISP_WIDTH, UI_DISP_HEIGHT - UI_STATUSBAR_H - UI_NAVBAR_H);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, UI_STATUSBAR_H);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 4, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    /* Left side: Terminal (60% width) */
    lv_obj_t* terminal_container = lv_obj_create(content);
    lv_obj_set_size(terminal_container, 280, UI_DISP_HEIGHT - UI_STATUSBAR_H - UI_NAVBAR_H - 12);
    lv_obj_align(terminal_container, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(terminal_container, UI_COLOR_BG_DARK, 0);
    lv_obj_set_style_border_width(terminal_container, 1, 0);
    lv_obj_set_style_border_color(terminal_container, UI_COLOR_PRIMARY, 0);
    lv_obj_set_style_radius(terminal_container, 4, 0);
    lv_obj_set_style_pad_all(terminal_container, 4, 0);
    lv_obj_clear_flag(terminal_container, LV_OBJ_FLAG_SCROLLABLE);

    /* Terminal header */
    lv_obj_t* term_header = lv_label_create(terminal_container);
    lv_label_set_text(term_header, "AVERROES TERMINAL");
    lv_obj_set_style_text_color(term_header, UI_COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(term_header, &lv_font_montserrat_10, 0);
    lv_obj_align(term_header, LV_ALIGN_TOP_MID, 0, 0);

    /* Terminal text area */
    terminal_ta = lv_textarea_create(terminal_container);
    lv_obj_set_size(terminal_ta, 268, UI_DISP_HEIGHT - UI_STATUSBAR_H - UI_NAVBAR_H - 36);
    lv_obj_align(terminal_ta, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(terminal_ta, &style_terminal, 0);
    lv_textarea_set_placeholder_text(terminal_ta, "> ");
    lv_textarea_set_text(terminal_ta, "");
    lv_textarea_set_one_line(terminal_ta, false);
    lv_obj_set_style_text_line_space(terminal_ta, 1, 0);
    lv_obj_set_style_text_font(terminal_ta, &lv_font_montserrat_10, 0);
    lv_group_add_obj(input_group, terminal_ta);

    /* Welcome message - compact */
    const char* welcome =
        "+----------------------+\n"
        "| AVERROES OS v1.0     |\n"
        "| T-LoRa Pager Ready   |\n"
        "+----------------------+\n"
        "> System OK\n"
        "> LoRa: Standby\n"
        "> GPS: Searching\n"
        "> WiFi: Off\n"
        "Ready.\n> ";
    lv_textarea_set_text(terminal_ta, welcome);

    /* Right side: Device Info (40% width) */
    lv_obj_t* info_container = lv_obj_create(content);
    lv_obj_set_size(info_container, 180, UI_DISP_HEIGHT - UI_STATUSBAR_H - UI_NAVBAR_H - 12);
    lv_obj_align(info_container, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(info_container, UI_COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(info_container, 1, 0);
    lv_obj_set_style_border_color(info_container, UI_COLOR_BORDER, 0);
    lv_obj_set_style_radius(info_container, 4, 0);
    lv_obj_set_style_pad_all(info_container, 6, 0);
    lv_obj_set_flex_flow(info_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(info_container, LV_OBJ_FLAG_SCROLLABLE);

    /* Device Info Header */
    lv_obj_t* info_header = lv_label_create(info_container);
    lv_label_set_text(info_header, LV_SYMBOL_LIST " DEVICE INFO");
    lv_obj_set_style_text_color(info_header, UI_COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(info_header, &lv_font_montserrat_10, 0);

    /* Device Name */
    lv_obj_t* dev_name = lv_label_create(info_container);
    lv_label_set_text(dev_name, "AVERROES-PAGER");
    lv_obj_set_style_text_color(dev_name, UI_COLOR_TEXT_BRIGHT, 0);
    lv_obj_set_style_text_font(dev_name, &lv_font_montserrat_12, 0);

    /* Hardware info */
    lv_obj_t* hw_info = lv_label_create(info_container);
    lv_label_set_text(hw_info,
        "MCU: ESP32-S3\n"
        "Flash: 16MB\n"
        "PSRAM: 8MB\n"
        "Display: 480x222\n"
        "Radio: SX1262");
    lv_obj_set_style_text_color(hw_info, UI_COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(hw_info, &lv_font_montserrat_10, 0);

    /* Version info */
    lv_obj_t* ver_info = lv_label_create(info_container);
    lv_label_set_text(ver_info,
        "\nFW: v1.0.0\n"
        "Build: " __DATE__);
    lv_obj_set_style_text_color(ver_info, UI_COLOR_SECONDARY, 0);
    lv_obj_set_style_text_font(ver_info, &lv_font_montserrat_10, 0);

    /* Navigation bar */
    ui_create_nav_bar(ui_screen_home);
}

/*═══════════════════════════════════════════════════════════════════════════
 * PAGE 2: CONNECTIVITY
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_setup_connectivity_page(void) {
    ui_screen_connectivity = lv_obj_create(NULL);
    lv_obj_add_style(ui_screen_connectivity, &style_screen, 0);
    lv_obj_set_flex_flow(ui_screen_connectivity, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_screen_connectivity, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    /* Status bar */
    ui_create_status_bar(ui_screen_connectivity);
    
    /* Scrollable content */
    lv_obj_t* content = lv_obj_create(ui_screen_connectivity);
    lv_obj_set_size(content, UI_DISP_WIDTH, UI_DISP_HEIGHT - UI_STATUSBAR_H - UI_NAVBAR_H);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, UI_STATUSBAR_H);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, UI_SPACING_SM, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    
    /* Title */
    lv_obj_t* title = lv_label_create(content);
    lv_label_set_text(title, UI_ICON_WIFI " CONNECTIVITY");
    lv_obj_add_style(title, &style_card_title, 0);
    
    /* WiFi Card */
    lv_obj_t* wifi_card = ui_create_card(content, "WiFi Networks");
    
    lv_obj_t* wifi_status = lv_label_create(wifi_card);
    lv_label_set_text(wifi_status, "Status: Disconnected");
    lv_obj_set_style_text_color(wifi_status, UI_COLOR_WARNING, 0);
    
    /* Buttons for future WiFi functionality */
    ui_create_icon_button(wifi_card, UI_ICON_REFRESH, "Scan");
    ui_create_icon_button(wifi_card, UI_ICON_WIFI, "Connect");
    
    /* LoRa Card */
    lv_obj_t* lora_card = ui_create_card(content, "LoRa Radio");
    
    lv_obj_t* lora_status = lv_label_create(lora_card);
    lv_label_set_text(lora_status, "Freq: 868.0 MHz\nMode: Standby\nRSSI: --");
    lv_obj_set_style_text_color(lora_status, UI_COLOR_TEXT_SECONDARY, 0);
    
    ui_create_switch_row(lora_card, "Enable LoRa", false);
    
    /* SSH Card */
    lv_obj_t* ssh_card = ui_create_card(content, "SSH Client");
    
    ui_create_switch_row(ssh_card, "WiFi Required", false);
    
    /* Button for future SSH functionality */
    ui_create_icon_button(ssh_card, LV_SYMBOL_KEYBOARD, "Connect SSH");
    
    /* Nav bar */
    ui_create_nav_bar(ui_screen_connectivity);
}

/*═══════════════════════════════════════════════════════════════════════════
 * PAGE 3: DISPLAY & THEMES
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_setup_display_page(void) {
    ui_screen_display = lv_obj_create(NULL);
    lv_obj_add_style(ui_screen_display, &style_screen, 0);
    
    /* Status bar */
    ui_create_status_bar(ui_screen_display);
    
    /* Scrollable content */
    lv_obj_t* content = lv_obj_create(ui_screen_display);
    lv_obj_set_size(content, UI_DISP_WIDTH, UI_DISP_HEIGHT - UI_STATUSBAR_H - UI_NAVBAR_H);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, UI_STATUSBAR_H);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, UI_SPACING_SM, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    
    /* Title */
    lv_obj_t* title = lv_label_create(content);
    lv_label_set_text(title, LV_SYMBOL_IMAGE " DISPLAY & THEMES");
    lv_obj_add_style(title, &style_card_title, 0);
    
    /* Brightness Card */
    lv_obj_t* bright_card = ui_create_card(content, "Brightness");
    ui_create_slider_row(bright_card, "Screen Brightness", 10, 255, ui_state.brightness);
    
    /* Theme Card */
    lv_obj_t* theme_card = ui_create_card(content, "Color Theme");
    
    /* Theme buttons grid */
    lv_obj_t* theme_grid = lv_obj_create(theme_card);
    lv_obj_set_size(theme_grid, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(theme_grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(theme_grid, 0, 0);
    lv_obj_set_style_pad_all(theme_grid, 0, 0);
    lv_obj_set_flex_flow(theme_grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(theme_grid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(theme_grid, LV_OBJ_FLAG_SCROLLABLE);
    
    const char* theme_names[] = {"Orange", "Cyan", "Magenta", "Green", "Amber"};
    lv_color_t theme_preview[] = {
        lv_color_hex(0xFF6B00),
        lv_color_hex(0x00FFAA),
        lv_color_hex(0xFF00AA),
        lv_color_hex(0x00FF00),
        lv_color_hex(0xFFAA00)
    };
    
    for(int i = 0; i < THEME_COUNT; i++) {
        lv_obj_t* btn = lv_btn_create(theme_grid);
        lv_obj_set_size(btn, 90, 36);
        lv_obj_add_style(btn, &style_btn, 0);
        lv_obj_add_style(btn, &style_btn_focused, LV_STATE_FOCUSED);
        lv_obj_set_style_bg_color(btn, theme_preview[i], 0);
        lv_obj_set_user_data(btn, (void*)(intptr_t)i);
        lv_group_add_obj(input_group, btn);
        
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, theme_names[i]);
        lv_obj_set_style_text_color(lbl, UI_COLOR_BG_DARK, 0);
        lv_obj_center(lbl);
        
        if(i == ui_state.currentTheme) {
            lv_obj_add_state(btn, LV_STATE_CHECKED);
            lv_obj_set_style_border_width(btn, 2, 0);
            lv_obj_set_style_border_color(btn, UI_COLOR_TEXT_BRIGHT, 0);
        }
    }
    
    /* Preview Card */
    lv_obj_t* preview_card = ui_create_card(content, "Preview");
    
    lv_obj_t* preview = lv_label_create(preview_card);
    lv_label_set_text(preview, "> AVERROES Terminal\n> Theme Preview\n> Ready._");
    lv_obj_set_style_text_color(preview, UI_COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(preview, &lv_font_montserrat_10, 0);
    
    /* Nav bar */
    ui_create_nav_bar(ui_screen_display);
}

/*═══════════════════════════════════════════════════════════════════════════
 * PAGE 4: SYSTEM & SENSORS
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_setup_system_page(void) {
    ui_screen_system = lv_obj_create(NULL);
    lv_obj_add_style(ui_screen_system, &style_screen, 0);
    
    /* Status bar */
    ui_create_status_bar(ui_screen_system);
    
    /* Scrollable content */
    lv_obj_t* content = lv_obj_create(ui_screen_system);
    lv_obj_set_size(content, UI_DISP_WIDTH, UI_DISP_HEIGHT - UI_STATUSBAR_H - UI_NAVBAR_H);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, UI_STATUSBAR_H);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, UI_SPACING_SM, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    
    /* Title */
    lv_obj_t* title = lv_label_create(content);
    lv_label_set_text(title, UI_ICON_SETTINGS " SYSTEM");
    lv_obj_add_style(title, &style_card_title, 0);
    
    /* Audio Card */
    lv_obj_t* audio_card = ui_create_card(content, "Audio");
    ui_create_switch_row(audio_card, "Sound Effects", ui_state.soundEnabled);
    ui_create_slider_row(audio_card, "Volume", 0, 100, ui_state.volume);
    
    /* Haptic Card */
    lv_obj_t* haptic_card = ui_create_card(content, "Haptic Feedback");
    ui_create_switch_row(haptic_card, "Enable Haptic", ui_state.hapticEnabled);
    ui_create_slider_row(haptic_card, "Intensity", 0, 100, ui_state.hapticIntensity);
    
    /* GPS Card */
    lv_obj_t* gps_card = ui_create_card(content, "GPS Status");
    
    lv_obj_t* gps_info = lv_label_create(gps_card);
    lv_label_set_text(gps_info, 
        "Fix: Searching...\n"
        "Lat: ---.------\n"
        "Lon: ---.------\n"
        "Sats: 0");
    lv_obj_set_style_text_color(gps_info, UI_COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(gps_info, &lv_font_montserrat_12, 0);
    
    /* Sensors Card */
    lv_obj_t* sensor_card = ui_create_card(content, "IMU Sensors");
    
    lv_obj_t* sensor_info = lv_label_create(sensor_card);
    lv_label_set_text(sensor_info, 
        "Accel: X:0 Y:0 Z:0\n"
        "Gyro:  X:0 Y:0 Z:0\n"
        "Steps: 0");
    lv_obj_set_style_text_color(sensor_info, UI_COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(sensor_info, &lv_font_montserrat_12, 0);
    
    /* Power Card */
    lv_obj_t* power_card = ui_create_card(content, "Power");
    
    lv_obj_t* power_info = lv_label_create(power_card);
    lv_label_set_text(power_info, 
        "Battery: 100%\n"
        "Voltage: 4.2V\n"
        "Status: Charged");
    lv_obj_set_style_text_color(power_info, UI_COLOR_SUCCESS, 0);
    lv_obj_set_style_text_font(power_info, &lv_font_montserrat_12, 0);
    
    /* System Actions Card */
    lv_obj_t* actions_card = ui_create_card(content, "Actions");
    
    lv_obj_t* btn_row = lv_obj_create(actions_card);
    lv_obj_set_size(btn_row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* btn_reset = ui_create_icon_button(btn_row, UI_ICON_REFRESH, "Reset");
    lv_obj_set_style_text_color(lv_obj_get_child(btn_reset, 0), UI_COLOR_WARNING, 0);
    
    lv_obj_t* btn_restart = ui_create_icon_button(btn_row, UI_ICON_POWER, "Restart");
    lv_obj_set_style_text_color(lv_obj_get_child(btn_restart, 0), UI_COLOR_ERROR, 0);
    
    /* Nav bar */
    ui_create_nav_bar(ui_screen_system);
}

/*═══════════════════════════════════════════════════════════════════════════
 * PAGE 5: INFO & DIAGNOSTICS
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_setup_info_page(void) {
    ui_screen_info = lv_obj_create(NULL);
    lv_obj_add_style(ui_screen_info, &style_screen, 0);
    
    /* Status bar */
    ui_create_status_bar(ui_screen_info);
    
    /* Scrollable content */
    lv_obj_t* content = lv_obj_create(ui_screen_info);
    lv_obj_set_size(content, UI_DISP_WIDTH, UI_DISP_HEIGHT - UI_STATUSBAR_H - UI_NAVBAR_H);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, UI_STATUSBAR_H);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, UI_SPACING_SM, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    
    /* Title */
    lv_obj_t* title = lv_label_create(content);
    lv_label_set_text(title, LV_SYMBOL_LIST " INFO & DIAGNOSTICS");
    lv_obj_add_style(title, &style_card_title, 0);
    
    /* About Card */
    lv_obj_t* about_card = ui_create_card(content, "About");
    
    lv_obj_t* logo = lv_label_create(about_card);
    lv_label_set_text(logo,
        "    ___ _   _\n"
        "   / _ \\ | | |\n"
        "  / /_\\ \\ | | |\n"
        "  |  _  | |_| |\n"
        "  |_| |_|\\___/\n"
        "  AVERROES");
    lv_obj_set_style_text_color(logo, UI_COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_12, 0);
    
    lv_obj_t* version = lv_label_create(about_card);
    lv_label_set_text(version, 
        "\nAVERROES OS v1.0.0\n"
        "T-LoRa Pager Firmware\n"
        "Build: " __DATE__);
    lv_obj_set_style_text_color(version, UI_COLOR_TEXT_SECONDARY, 0);
    
    /* Hardware Card */
    lv_obj_t* hw_card = ui_create_card(content, "Hardware");
    
    /* Display shown as 480x222 (landscape) - physical is 222x480 (portrait) */
    lv_obj_t* hw_info = lv_label_create(hw_card);
    lv_label_set_text(hw_info, 
        "MCU: ESP32-S3\n"
        "Flash: 16MB\n"
        "PSRAM: 8MB\n"
        "Display: 480x222\n"
        "Radio: SX1262 LoRa\n"
        "GPS: u-blox MIA-M10Q\n"
        "IMU: BHI260AP\n"
        "Audio: ES8311\n"
        "NFC: ST25R3916");
    lv_obj_set_style_text_color(hw_info, UI_COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(hw_info, &lv_font_montserrat_12, 0);
    
    /* Memory Card */
    lv_obj_t* mem_card = ui_create_card(content, "Memory");
    
    lv_obj_t* mem_info = lv_label_create(mem_card);
    lv_label_set_text(mem_info, 
        "Heap Free: ---KB\n"
        "PSRAM Free: ---KB\n"
        "Flash Used: ---KB");
    lv_obj_set_style_text_color(mem_info, UI_COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(mem_info, &lv_font_montserrat_12, 0);
    
    /* Links Card */
    lv_obj_t* links_card = ui_create_card(content, "Links");
    
    lv_obj_t* links = lv_label_create(links_card);
    lv_label_set_text(links, 
        "GitHub:\n"
        "averroes-tech/t-lora-pager\n\n"
        "Averroes Tech Manufacturing\n"
        "Kuwait");
    lv_obj_set_style_text_color(links, UI_COLOR_SECONDARY, 0);
    lv_obj_set_style_text_font(links, &lv_font_montserrat_12, 0);
    
    /* Nav bar */
    ui_create_nav_bar(ui_screen_info);
}

/*═══════════════════════════════════════════════════════════════════════════
 * INPUT HANDLING
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_handle_encoder_rotate(int8_t diff) {
    if(diff > 0) {
        lv_group_focus_next(input_group);
    } else if(diff < 0) {
        lv_group_focus_prev(input_group);
    }
}

void ui_handle_encoder_click(void) {
    lv_obj_t* focused = lv_group_get_focused(input_group);
    if(focused) {
        lv_obj_send_event(focused, LV_EVENT_CLICKED, NULL);
    }
}

void ui_handle_encoder_long_press(void) {
    /* Return to home page on long press */
    ui_load_page_anim(PAGE_HOME, LV_SCR_LOAD_ANIM_FADE_IN);
}

/* Adjust slider value for focused slider object */
static void adjust_focused_slider(int32_t delta) {
    lv_obj_t* focused = lv_group_get_focused(input_group);
    if(focused && lv_obj_check_type(focused, &lv_slider_class)) {
        int32_t val = lv_slider_get_value(focused);
        int32_t min_val = lv_slider_get_min_value(focused);
        int32_t max_val = lv_slider_get_max_value(focused);
        int32_t step = (max_val - min_val) / 20;  /* 5% steps */
        if(step < 1) step = 1;
        val += delta * step;
        if(val < min_val) val = min_val;
        if(val > max_val) val = max_val;
        lv_slider_set_value(focused, val, LV_ANIM_ON);
        lv_obj_send_event(focused, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

void ui_handle_key_press(uint8_t key) {
    /* ═══════════════════════════════════════════════════════════════════
     * QUICK PAGE NAVIGATION (Number Keys 1-5)
     * ═══════════════════════════════════════════════════════════════════*/
    if(key >= '1' && key <= '5') {
        /* Only if not in terminal text entry mode */
        if(!terminal_ta || !lv_obj_has_state(terminal_ta, LV_STATE_FOCUSED)) {
            ui_page_t page = (ui_page_t)(key - '1');
            ui_load_page_anim(page, LV_SCR_LOAD_ANIM_FADE_IN);
            return;
        }
    }

    /* ═══════════════════════════════════════════════════════════════════
     * ESCAPE / BACK KEYS (ESC, B, $)
     * ═══════════════════════════════════════════════════════════════════*/
    if(key == 27 || key == '$') {  /* ESC or $ (back symbol) */
        if(!terminal_ta || !lv_obj_has_state(terminal_ta, LV_STATE_FOCUSED)) {
            ui_load_page(PAGE_HOME);
            return;
        }
    }

    /* ═══════════════════════════════════════════════════════════════════
     * NAVIGATION KEYS (W/S = Up/Down, A/D = Left/Right or Adjust)
     * ═══════════════════════════════════════════════════════════════════*/
    if(key == 'w' || key == 'W') {
        /* Navigate up / previous item */
        if(!terminal_ta || !lv_obj_has_state(terminal_ta, LV_STATE_FOCUSED)) {
            lv_group_focus_prev(input_group);
            return;
        }
    }

    if(key == 's' || key == 'S') {
        /* Navigate down / next item */
        if(!terminal_ta || !lv_obj_has_state(terminal_ta, LV_STATE_FOCUSED)) {
            lv_group_focus_next(input_group);
            return;
        }
    }

    if(key == 'a' || key == 'A') {
        /* Left / Decrease value for sliders */
        if(!terminal_ta || !lv_obj_has_state(terminal_ta, LV_STATE_FOCUSED)) {
            adjust_focused_slider(-1);
            return;
        }
    }

    if(key == 'd' || key == 'D') {
        /* Right / Increase value for sliders */
        if(!terminal_ta || !lv_obj_has_state(terminal_ta, LV_STATE_FOCUSED)) {
            adjust_focused_slider(1);
            return;
        }
    }

    /* ═══════════════════════════════════════════════════════════════════
     * TAB KEY - Next element
     * ═══════════════════════════════════════════════════════════════════*/
    if(key == '\t' || key == 9) {
        lv_group_focus_next(input_group);
        return;
    }

    /* ═══════════════════════════════════════════════════════════════════
     * ENTER - Select/Click focused element (when not in terminal)
     * ═══════════════════════════════════════════════════════════════════*/
    if(key == '\r' || key == '\n') {
        if(!terminal_ta || !lv_obj_has_state(terminal_ta, LV_STATE_FOCUSED)) {
            ui_handle_encoder_click();
            return;
        }
    }

    /* ═══════════════════════════════════════════════════════════════════
     * SPACE - Select/Click focused element (when not in terminal)
     * ═══════════════════════════════════════════════════════════════════*/
    if(key == ' ') {
        if(!terminal_ta || !lv_obj_has_state(terminal_ta, LV_STATE_FOCUSED)) {
            ui_handle_encoder_click();
            return;
        }
    }

    /* ═══════════════════════════════════════════════════════════════════
     * FUNCTION KEYS (via keyboard shortcuts)
     * H = Home, N = Network, T = Theme, Y = System, I = Info
     * ═══════════════════════════════════════════════════════════════════*/
    if(!terminal_ta || !lv_obj_has_state(terminal_ta, LV_STATE_FOCUSED)) {
        switch(key) {
            case 'h': case 'H': ui_load_page(PAGE_HOME); return;
            case 'n': case 'N': ui_load_page(PAGE_CONNECTIVITY); return;
            case 't': case 'T': ui_load_page(PAGE_DISPLAY); return;
            case 'y': case 'Y': ui_load_page(PAGE_SYSTEM); return;
            case 'i': case 'I': ui_load_page(PAGE_INFO); return;
        }
    }

    /* ═══════════════════════════════════════════════════════════════════
     * TERMINAL TEXT INPUT (when terminal is focused)
     * ═══════════════════════════════════════════════════════════════════*/
    if(terminal_ta && lv_obj_has_state(terminal_ta, LV_STATE_FOCUSED)) {
        if(key == '\r' || key == '\n') {
            /* Enter - new line and prompt */
            lv_textarea_add_char(terminal_ta, '\n');
            lv_textarea_add_text(terminal_ta, "> ");
        } else if(key == 8 || key == 127) {
            /* Backspace or DEL */
            lv_textarea_delete_char(terminal_ta);
        } else if(key >= 32 && key <= 126) {
            /* Printable ASCII characters */
            lv_textarea_add_char(terminal_ta, key);
        }
    }
}

/*═══════════════════════════════════════════════════════════════════════════
 * STATUS UPDATE FUNCTIONS
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_update_wifi_status(bool connected, int8_t rssi) {
    if(!lbl_wifi_icon) return;
    lv_obj_set_style_text_color(lbl_wifi_icon, 
        connected ? UI_COLOR_SUCCESS : UI_COLOR_TEXT_SECONDARY, 0);
}

void ui_update_lora_status(bool active, int8_t rssi) {
    if(!lbl_lora_icon) return;
    lv_obj_set_style_text_color(lbl_lora_icon,
        active ? UI_COLOR_SUCCESS : UI_COLOR_TEXT_SECONDARY, 0);
}

void ui_update_gps_status(bool fix, uint8_t satellites) {
    if(!lbl_gps_icon) return;
    lv_obj_set_style_text_color(lbl_gps_icon,
        fix ? UI_COLOR_SUCCESS : UI_COLOR_WARNING, 0);
}

void ui_update_battery(uint8_t percent, bool charging) {
    if(!lbl_battery_icon) return;
    
    char buf[20];
    const char* icon = UI_ICON_BATTERY;
    if(charging) icon = UI_ICON_CHARGE;
    
    snprintf(buf, sizeof(buf), "%s %d%%", icon, percent);
    lv_label_set_text(lbl_battery_icon, buf);
    
    lv_color_t color;
    if(percent > 60) color = UI_COLOR_SUCCESS;
    else if(percent > 20) color = UI_COLOR_WARNING;
    else color = UI_COLOR_ERROR;
    
    lv_obj_set_style_text_color(lbl_battery_icon, color, 0);
}

void ui_update_terminal(const char* text) {
    if(terminal_ta) {
        lv_textarea_set_text(terminal_ta, text);
    }
}

void ui_append_terminal(const char* text) {
    if(terminal_ta) {
        lv_textarea_add_text(terminal_ta, text);
        lv_textarea_cursor_down(terminal_ta);
    }
}

void ui_clear_terminal(void) {
    if(terminal_ta) {
        lv_textarea_set_text(terminal_ta, "> ");
    }
}

/*═══════════════════════════════════════════════════════════════════════════
 * ANIMATION HELPERS
 *═══════════════════════════════════════════════════════════════════════════*/
static void anim_opa_cb(void* var, int32_t v) {
    lv_obj_set_style_opa((lv_obj_t*)var, v, 0);
}

void ui_anim_fade_in(lv_obj_t* obj, uint32_t delay) {
    lv_obj_set_style_opa(obj, LV_OPA_TRANSP, 0);
    
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&a, 300);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, anim_opa_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

static void anim_x_cb(void* var, int32_t v) {
    lv_obj_set_x((lv_obj_t*)var, v);
}

void ui_anim_slide_in(lv_obj_t* obj, lv_dir_t dir, uint32_t delay) {
    int32_t start_x = 0;
    if(dir == LV_DIR_LEFT) start_x = -UI_DISP_WIDTH;
    else if(dir == LV_DIR_RIGHT) start_x = UI_DISP_WIDTH;
    
    lv_obj_set_x(obj, start_x);
    
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, start_x, 0);
    lv_anim_set_time(&a, 300);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

/*═══════════════════════════════════════════════════════════════════════════
 * THEME MANAGEMENT
 *═══════════════════════════════════════════════════════════════════════════*/
void ui_set_theme(ui_theme_t theme) {
    if(theme >= THEME_COUNT) {
        theme = THEME_CYBERPUNK_ORANGE;
    }
    ui_state.currentTheme = theme;
    ui_apply_theme();
}

void ui_apply_theme(void) {
    /* Theme colors are already defined in theme_colors array */
    /* Future: Apply theme colors to styles and refresh UI */
}

void ui_update_status_bar(void) {
    /* Future: Refresh all status bar elements */
    /* Currently handled by individual update functions */
}

void ui_anim_pulse(lv_obj_t* obj) {
    if(!obj) return;
    /* Opacity pulse animation - fades object and back for attention effect */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_50);
    lv_anim_set_time(&a, 200);
    lv_anim_set_playback_time(&a, 200);
    lv_anim_set_exec_cb(&a, anim_opa_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}

void ui_anim_glow(lv_obj_t* obj, lv_color_t color) {
    if(!obj) return;
    /* Set shadow glow effect */
    lv_obj_set_style_shadow_width(obj, 16, 0);
    lv_obj_set_style_shadow_color(obj, color, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_70, 0);
}
