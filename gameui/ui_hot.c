#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

// ---------------- 常量 ----------------
#define SWEAT_SPEED 2
#define SWEAT_OFFSET_MAX 50

// ---------------- 全局变量 ----------------
lv_obj_t* sweat[10];
int offset[10];
lv_obj_t* orange_overlay = NULL;
lv_timer_t* sweat_timer = NULL;
bool sweat_enabled = false;

// 汗滴图片
lv_img_dsc_t sweat_img_dsc;
uint8_t* sweat_image_buffer = NULL;

// ---------------- 私有函数 ----------------
void init_sweat_image(void) {
    sweat_image_buffer = sdram_malloc(10 * 15 * 3 + 4);
    if(!sweat_image_buffer) return;
    read_file_to_array("0:/sweat.bin", sweat_image_buffer, 10 * 15 * 3 + 4);

    sweat_img_dsc.header.always_zero = 0;
    sweat_img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    sweat_img_dsc.header.w = 10;
    sweat_img_dsc.header.h = 15 ;
    sweat_img_dsc.header.reserved = 0;
    sweat_img_dsc.data_size = 10 * 15 * 3;
    sweat_img_dsc.data = sweat_image_buffer + 4;
}

// 汗滴动画回调
static void sweat_timer_cb(lv_timer_t* timer) {
    if(!sweat_enabled) return;

    for(int i = 0; i < 10; i++) {
        int pig_x = pig_fsms[i].pig_t.x;
        int pig_y = pig_fsms[i].pig_t.y;

        offset[i] += SWEAT_SPEED;
        if(offset[i] > 20)
           offset[i] = 0;

        if(sweat[i])
            lv_obj_set_pos(sweat[i],
            pig_x + 60,
            pig_y + 15 + offset[i]);
    }

    // 只需要置顶橙色叠加和汗滴
    if(orange_overlay) lv_obj_move_foreground(orange_overlay);
    for(int i = 0; i < 10; i++) {
        if(sweat[i])
            lv_obj_move_foreground(sweat[i]);
    }
}

// ---------------- 公共接口 ----------------
void hot_start(void) {
    if(sweat_enabled) return;
    sweat_enabled = true;

    init_sweat_image(); // 初始化汗滴图片

    // 橙色叠加（透明度调低一点）
    orange_overlay = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(orange_overlay);
    lv_obj_set_size(orange_overlay, 1024, 600);
    lv_obj_set_style_bg_color(orange_overlay, lv_color_hex(0xFFAA33), 0);
    lv_obj_set_style_bg_opa(orange_overlay, LV_OPA_30, 0); // 比之前更透明
    lv_obj_clear_flag(orange_overlay, LV_OBJ_FLAG_CLICKABLE);

    // 创建每只猪的汗滴
    for(int i = 0; i < 10; i++) {
        sweat[i] = lv_img_create(lv_scr_act());
        lv_img_set_src(sweat[i], &sweat_img_dsc);
        offset[i] = 0;
    }

    // 定时器动画
    sweat_timer = lv_timer_create(sweat_timer_cb, 50, NULL);
}

void hot_stop(void) {
    if(!sweat_enabled) return;
    sweat_enabled = false;

    if(sweat_timer) {
        lv_timer_del(sweat_timer);
        sweat_timer = NULL;
    }

    if(orange_overlay) { lv_obj_del(orange_overlay); orange_overlay = NULL; }

    for(int i = 0; i < 10; i++) {
        if(sweat[i]) {
            lv_obj_del(sweat[i]);
            sweat[i] = NULL;
        }
    }

    if(sweat_image_buffer) {
        free(sweat_image_buffer);
        sweat_image_buffer = NULL;
    }

    lv_obj_invalidate(lv_scr_act());
}