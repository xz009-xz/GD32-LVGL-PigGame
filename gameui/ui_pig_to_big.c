#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

static lv_obj_t *target_pig = NULL;
static lv_obj_t *dark_bg = NULL;
static lv_obj_t *circle_cover = NULL;

static int flash_count = 0;

// 配置参数
#define SMALL_FLASH_MAX    10
#define SMALL_FLASH_SPEED  120
#define BIG_FLASH_MAX      10
#define BIG_FLASH_SPEED    80
#define ANIM_DELAY         500
#define BIG_STAY_TIME      600

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
}

static void anim_all_finish(void)
{
    lv_obj_clear_flag(target_pig, LV_OBJ_FLAG_HIDDEN);
    restore_screen();
}

// 大猪闪烁
static void anim_big_pig_flash(lv_anim_t *anim)
{
    static int cnt = 0;

    if (lv_obj_has_flag(target_pig, LV_OBJ_FLAG_HIDDEN))
        lv_obj_clear_flag(target_pig, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(target_pig, LV_OBJ_FLAG_HIDDEN);

    cnt++;
    if(cnt >= BIG_FLASH_MAX)
    {
        cnt = 0;
        anim_all_finish();
        return;
    }

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, target_pig);
    lv_anim_set_time(&a, BIG_FLASH_SPEED);
    lv_anim_set_ready_cb(&a, anim_big_pig_flash);
    lv_anim_start(&a);
}

// 停留结束 → 大猪隐藏，开始闪烁
static void anim_big_start_flash(lv_anim_t *anim)
{
    lv_obj_add_flag(target_pig, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, target_pig);
    lv_anim_set_time(&a, BIG_FLASH_SPEED);
    lv_anim_set_ready_cb(&a, anim_big_pig_flash);
    lv_anim_start(&a);
}

// 小猪闪完 → 隐藏 → 变大猪 → 显示 → 停留
static void anim_pig_final(void)
{
    lv_obj_add_flag(target_pig, LV_OBJ_FLAG_HIDDEN);
    lv_img_set_src(target_pig, &pig_fsms[0].pig_t.image_pig_big);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, target_pig);
    lv_anim_set_time(&a, 0);
    lv_anim_set_delay(&a, BIG_STAY_TIME);
    lv_anim_set_ready_cb(&a, anim_big_start_flash);
    lv_anim_start(&a);
}

// 小猪闪烁
static void anim_small_pig_flash(lv_anim_t *anim)
{
    flash_count++;
    if (flash_count >= SMALL_FLASH_MAX) {
        anim_pig_final();
        return;
    }

    if (lv_obj_has_flag(target_pig, LV_OBJ_FLAG_HIDDEN))
        lv_obj_clear_flag(target_pig, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(target_pig, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, target_pig);
    lv_anim_set_time(&a, SMALL_FLASH_SPEED);
    lv_anim_set_ready_cb(&a, anim_small_pig_flash);
    lv_anim_start(&a);
}

void pig_grow_anim(int pig_idx)
{
	pig_idx=0;//测试用！不能删
    target_pig = pig_fsms[pig_idx].pig_t.img_pig;
    flash_count = 0;

    // 全屏遮罩（已修正变量名）
    dark_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dark_bg, 1024 + 100, 600 + 100);
    lv_obj_align(dark_bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(dark_bg, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(dark_bg, 160, 0);
    lv_obj_clear_flag(dark_bg, LV_OBJ_FLAG_SCROLLABLE);

    // 中间透明区域
    int x = pig_fsms[pig_idx].pig_t.x;
    int y = pig_fsms[pig_idx].pig_t.y;
    int r = 90;

    circle_cover = lv_obj_create(lv_scr_act());
    lv_obj_set_size(circle_cover, r*2, r*2);
    lv_obj_align(circle_cover, LV_ALIGN_TOP_LEFT, x - r, y - r);
    lv_obj_set_style_radius(circle_cover, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(circle_cover, 0, 0);
    lv_obj_set_style_border_width(circle_cover, 0, 0);

    lv_obj_move_foreground(circle_cover);
    lv_obj_move_foreground(target_pig);

    lv_obj_add_flag(target_pig, LV_OBJ_FLAG_HIDDEN);

    // 启动动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, target_pig);
    lv_anim_set_time(&a, SMALL_FLASH_SPEED);
    lv_anim_set_delay(&a, ANIM_DELAY);
    lv_anim_set_ready_cb(&a, anim_small_pig_flash);
    lv_anim_start(&a);
}
