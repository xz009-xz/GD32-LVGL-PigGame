#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

static lv_obj_t *target_pig = NULL;
static lv_obj_t *food_obj = NULL;
static lv_obj_t *dark_bg = NULL;
static lv_obj_t *circle_cover = NULL;

static int flash_count = 0;
static int base_food_x = 0;
static int base_food_y = 0;

// ===================== 可调节参数 =====================
#define FOOD_FLASH_MAX      10
#define FOOD_FLASH_SPEED    120
#define FOOD_SHAKE_OFFSET   6
#define FOOD_OPA_STEP       25
#define ANIM_DELAY          500
#define FOOD_OFFSET_X       20
#define FOOD_OFFSET_Y       40
// ======================================================

static void restore_screen(void)
{
    if(dark_bg) {
        lv_obj_del(dark_bg);
        dark_bg = NULL;
    }
    if(circle_cover) {
        lv_obj_del(circle_cover);
        circle_cover = NULL;
    }
    if(food_obj) {
        lv_obj_del(food_obj);
        food_obj = NULL;
    }
}

static void anim_all_finish(void)
{
    lv_obj_clear_flag(target_pig, LV_OBJ_FLAG_HIDDEN);
    restore_screen();
}

static void anim_food_flash_shake(lv_anim_t *anim)
{
    flash_count++;
    if(flash_count >= FOOD_FLASH_MAX)
    {
        anim_all_finish();
        return;
    }

    int shake_x = (flash_count % 2 == 0) ? FOOD_SHAKE_OFFSET : -FOOD_SHAKE_OFFSET;
    int shake_y = (flash_count % 2 == 0) ? FOOD_SHAKE_OFFSET : -FOOD_SHAKE_OFFSET;

    lv_obj_set_x(food_obj, base_food_x + shake_x);
    lv_obj_set_y(food_obj, base_food_y + shake_y);

    uint8_t now_opa = lv_obj_get_style_opa(food_obj, 0);
    if(now_opa > FOOD_OPA_STEP)
        lv_obj_set_style_opa(food_obj, now_opa - FOOD_OPA_STEP, 0);

    // 第一次不隐藏！！！
    if(flash_count > 1)
    {
        if(lv_obj_has_flag(food_obj, LV_OBJ_FLAG_HIDDEN))
            lv_obj_clear_flag(food_obj, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(food_obj, LV_OBJ_FLAG_HIDDEN);
    }

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, food_obj);
    lv_anim_set_time(&a, FOOD_FLASH_SPEED);
    lv_anim_set_ready_cb(&a, anim_food_flash_shake);
    lv_anim_start(&a);
}

void pig_feed_anim(int pig_idx, int fruit_idx)
{
	
    target_pig = pig_fsms[pig_idx].pig_t.img_pig;
    flash_count = 0;

    // 暗场
    dark_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dark_bg, 1024+100, 600+100);
    lv_obj_align(dark_bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(dark_bg, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(dark_bg, 160, 0);
    lv_obj_clear_flag(dark_bg, LV_OBJ_FLAG_SCROLLABLE);

    int x = pig_fsms[pig_idx].pig_t.x;
    int y = pig_fsms[pig_idx].pig_t.y;
    int r = 90;

    // 光圈
    circle_cover = lv_obj_create(lv_scr_act());
    lv_obj_set_size(circle_cover, r*2, r*2);
    lv_obj_align(circle_cover, LV_ALIGN_TOP_LEFT, x - r, y - r);
    lv_obj_set_style_radius(circle_cover, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(circle_cover, 0, 0);
    lv_obj_set_style_border_width(circle_cover, 0, 0);

    // ===================== 食物 =====================
    food_obj = lv_img_create(lv_scr_act());
    lv_img_set_src(food_obj, &foods[fruit_idx].img);
    
    base_food_x = x + FOOD_OFFSET_X;
    base_food_y = y + FOOD_OFFSET_Y;
    
    lv_obj_set_x(food_obj, base_food_x);
    lv_obj_set_y(food_obj, base_food_y);
    lv_obj_set_style_opa(food_obj, 255, 0);

    // ========== 终极修复：食物永远在最顶层 ==========
    lv_obj_move_foreground(dark_bg);
    lv_obj_move_foreground(circle_cover);
    lv_obj_move_foreground(target_pig);
    lv_obj_move_foreground(food_obj); // 最后抬食物

    // 启动动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, food_obj);
    lv_anim_set_time(&a, FOOD_FLASH_SPEED);
    lv_anim_set_delay(&a, ANIM_DELAY);
    lv_anim_set_ready_cb(&a, anim_food_flash_shake);
    lv_anim_start(&a);
}
