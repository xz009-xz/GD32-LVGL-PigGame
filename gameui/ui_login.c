#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"
#include "gamelogic/disaster.h"
#include "gamelogic/auth.h"
#include "gamelogic/save.h"

static lv_obj_t *login_panel;
static lv_obj_t *username_ta;
static lv_obj_t *password_ta;
static lv_obj_t *keyboard;
static lv_obj_t *remember_btn;
static bool remember_me = false;
static bool remember_file_exists = false;

static void update_remember_button_style(void)
{
    if (remember_file_exists) {
        lv_obj_set_style_bg_color(remember_btn, lv_color_hex(0xFF0000), 0);
    } else {
        lv_obj_set_style_bg_color(remember_btn, lv_color_hex(0x999999), 0);
    }
}

// 只留默认处理，不重复触发
static void keyboard_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    // 不写任何东西，让键盘自己正常工作
}

static void input_click_cb(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    lv_keyboard_set_textarea(keyboard, ta);
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}

static void panel_click_cb(lv_event_t *e)
{
    if(lv_event_get_target(e) == login_panel)
    {
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

static void login_btn_cb(lv_event_t *e);
static void reg_btn_cb(lv_event_t *e);

static void remember_btn_cb(lv_event_t *e)
{
    if (remember_file_exists) {
        auth_clear_remember();
        remember_file_exists = false;
        remember_me = false;
        update_remember_button_style();
        return;
    }

    const char *username = lv_textarea_get_text(username_ta);
    const char *password = lv_textarea_get_text(password_ta);
    if (username[0] == '\0' || password[0] == '\0') {
        return;
    }

    if (auth_save_remember(username, password) == AUTH_OK) {
        remember_file_exists = true;
        remember_me = true;
        update_remember_button_style();
    }
}

static void login_btn_cb(lv_event_t *e) {
    const char *username = lv_textarea_get_text(username_ta);
    const char *password = lv_textarea_get_text(password_ta);

    AuthResult res = auth_login(username, password);

    switch (res) {
        case AUTH_OK: {
            // 记录当前用户
            extern char current_user[];
            strncpy(current_user, username, MAX_USERNAME_LEN);
            current_user[MAX_USERNAME_LEN] = '\0';

            // 只有当用户显式点击记住我才保存当前登录信息
            if (remember_me) {
                auth_save_remember(username, password);
                remember_file_exists = true;
            }

            // 进入游戏 (ui_game_screen 内部会自动 load 存档)
            ui_game_screen(NULL);
            break;
        }
        case AUTH_ERR_USER_NOT_FOUND:
            // 显示用户不存在
            break;
        case AUTH_ERR_WRONG_PASSWORD:
            // 显示密码错误
            break;
        default:
            break;
    }
}

static void reg_btn_cb(lv_event_t *e){

    const char *username = lv_textarea_get_text(username_ta);
    const char *password = lv_textarea_get_text(password_ta);

    AuthResult res = auth_register(username, password);

    switch(res){
        case AUTH_OK: {
            // 记录当前用户
            extern char current_user[];
            strncpy(current_user, username, MAX_USERNAME_LEN);
            current_user[MAX_USERNAME_LEN] = '\0';

            if(remember_me){
                auth_save_remember(username, password);
                remember_file_exists = true;
            }

            // 新用户直接进入游戏（无需加载存档）
            ui_game_screen(NULL);
            break;
        }
        case AUTH_ERR_USER_EXIST:
            // 显示用户已存在错误
            break;
        default:
            // 显示其他错误
            break;
    }
}

void create_login_ui(void)
{
    login_panel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(login_panel, 520, 380);
    lv_obj_center(login_panel);
    lv_obj_set_style_bg_color(login_panel, lv_color_hex(0xD7B447), 0);
    lv_obj_set_style_bg_opa(login_panel, LV_OPA_90, 0);
    lv_obj_set_style_border_width(login_panel, 3, 0);
    lv_obj_set_style_border_color(login_panel, lv_color_white(), 0);
    lv_obj_set_style_radius(login_panel, 0, 0);
    lv_obj_add_event_cb(login_panel, panel_click_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *title = lv_label_create(login_panel);
    lv_label_set_text(title, "LOGIN");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_t *user_label = lv_label_create(login_panel);
    lv_label_set_text(user_label, "Username");
    lv_obj_set_style_text_color(user_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(user_label, &lv_font_montserrat_24, 0);
    lv_obj_align(user_label, LV_ALIGN_TOP_MID, 0, 80);

    username_ta = lv_textarea_create(login_panel);
    lv_obj_set_size(username_ta, 360, 45);
    lv_obj_align(username_ta, LV_ALIGN_TOP_MID, 0, 110);
    lv_textarea_set_one_line(username_ta, true);
    lv_textarea_set_max_length(username_ta, 31);
    lv_obj_add_event_cb(username_ta, input_click_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *pass_label = lv_label_create(login_panel);
    lv_label_set_text(pass_label, "Password");
    lv_obj_set_style_text_color(pass_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(pass_label, &lv_font_montserrat_24, 0);
    lv_obj_align(pass_label, LV_ALIGN_TOP_MID, 0, 170);

    password_ta = lv_textarea_create(login_panel);
    lv_obj_set_size(password_ta, 360, 45);
    lv_obj_align(password_ta, LV_ALIGN_TOP_MID, 0, 200);
    lv_textarea_set_one_line(password_ta, true);
    lv_textarea_set_password_mode(password_ta, true);
    lv_textarea_set_max_length(password_ta, 31);
    lv_obj_add_event_cb(password_ta, input_click_cb, LV_EVENT_CLICKED, NULL);

    remember_btn = lv_btn_create(login_panel);
    lv_obj_set_size(remember_btn, 20, 20);
    lv_obj_align(remember_btn, LV_ALIGN_TOP_LEFT, 80, 255);
    lv_obj_set_style_radius(remember_btn, 3, 0);
    lv_obj_set_style_bg_color(remember_btn, lv_color_hex(0x999999), 0);
    lv_obj_add_event_cb(remember_btn, remember_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *rem_label = lv_label_create(login_panel);
    lv_label_set_text(rem_label, "Remember me");
    lv_obj_set_style_text_color(rem_label, lv_color_white(), 0);
    lv_obj_align_to(rem_label, remember_btn, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t *login_btn = lv_btn_create(login_panel);
    lv_obj_set_size(login_btn, 150, 45);
    lv_obj_align(login_btn, LV_ALIGN_BOTTOM_LEFT, 60, -5);
    lv_obj_t *l_text = lv_label_create(login_btn);
    lv_label_set_text(l_text, "LOGIN");
    lv_obj_center(l_text);
    lv_obj_add_event_cb(login_btn, login_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *reg_btn = lv_btn_create(login_panel);
    lv_obj_set_size(reg_btn, 150, 45);
    lv_obj_align(reg_btn, LV_ALIGN_BOTTOM_RIGHT, -60, -5);
    lv_obj_t *r_text = lv_label_create(reg_btn);
    lv_label_set_text(r_text, "REGISTER");
    lv_obj_center(r_text);
    lv_obj_add_event_cb(reg_btn, reg_btn_cb, LV_EVENT_CLICKED, NULL);

    // 纯原生键盘，不做任何多余处理
    keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(keyboard, 1024, 240);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    // 只挂一个空回调，避免重复执行
    lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    char rem_user[MAX_USERNAME_LEN + 1] = {0};
    char rem_pass[MAX_PASSWORD_LEN + 1] = {0};
    if (auth_load_remember(rem_user, rem_pass, sizeof(rem_user)) == AUTH_OK) {
        lv_textarea_set_text(username_ta, rem_user);
        lv_textarea_set_text(password_ta, rem_pass);
        remember_file_exists = true;
        remember_me = false;
        update_remember_button_style();
    }
}
void save_btn_cb(lv_event_t *e) {
    extern char current_user[];
    if (current_user[0] != '\0') {
        if (save_game(current_user)) {
            // 显示 "Saved!" 提示
            lv_obj_t *msg = lv_label_create(lv_scr_act());
            lv_label_set_text(msg, "Saved!");
            lv_obj_set_style_text_color(msg, lv_color_hex(0x00FF00), 0);
            lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 50);
            // 2 秒后自动消失 (可用 lv_timer 实现)
        }
    }
}
