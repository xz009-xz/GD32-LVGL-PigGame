#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

// ---------------- 全局变量 ----------------
lv_obj_t* sweat[10];
lv_obj_t* orange_overlay = NULL;
bool sweat_enabled = false;

// 汗滴图片
lv_img_dsc_t sweat_img_dsc;
uint8_t* sweat_image_buffer = NULL;

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
static void sweat_anim_cb(void * obj, int32_t v)
{
    lv_obj_t * img = (lv_obj_t *)obj;
    lv_obj_set_y(img, 15 + v);
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
    lv_obj_set_style_bg_opa(orange_overlay, LV_OPA_30, 0);
    lv_obj_clear_flag(orange_overlay, LV_OBJ_FLAG_CLICKABLE);

    // 创建每只猪的汗滴
    for(int i = 0; i < 10; i++) {
        if(!pig_fsms[i].pig_t.img_pig) continue;

        // 创建汗滴为猪对象的子对象
        sweat[i] = lv_img_create(pig_fsms[i].pig_t.img_pig);
        lv_img_set_src(sweat[i], &sweat_img_dsc);

        // 初始位置（相对父对象）
        lv_obj_set_pos(sweat[i], 60, 15);

        // 动画
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, sweat[i]);
        lv_anim_set_exec_cb(&a, sweat_anim_cb);
        lv_anim_set_values(&a, 0, 20);
        lv_anim_set_time(&a, 1600);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&a);

        lv_obj_move_foreground(pig_fsms[i].pig_t.img_pig);
    }
}

void hot_stop(void) {
    if(!sweat_enabled) return;
    sweat_enabled = false;

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

}
