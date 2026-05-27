#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

// =========================
// 雪花配置
// =========================

#define SNOW_COUNT 35

typedef struct {
    int x;
    int y;
    int size;
    int speed;
    int drift;
} snowflake_t;

static snowflake_t snowflakes[SNOW_COUNT];

// =========================
// 图片资源
// =========================

lv_img_dsc_t snow_main_dsc;
lv_img_dsc_t snow_big_dsc;
lv_img_dsc_t snow_small_dsc;

extern lv_obj_t *img_main;
extern lv_img_dsc_t image_struct;

// =========================
// 定时器
// =========================

static lv_timer_t *snow_timer = NULL;
static lv_timer_t *pig_check_timer = NULL;

// =========================
// 创建雪天图片
// =========================

void create_snow_image(void)
{
    uint8_t* snow_main_buffer =
        sdram_malloc(1024 * 600 * 3 + 4);

    if(!snow_main_buffer) return;

    read_file_to_array(
        "0:/snow_main.bin",
        snow_main_buffer,
        1024 * 600 * 3 + 4
    );

    snow_main_dsc.header.always_zero = 0;
    snow_main_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    snow_main_dsc.header.w = 1024;
    snow_main_dsc.header.h = 600;
    snow_main_dsc.header.reserved = 0;
    snow_main_dsc.data_size = 1024 * 600 * 3;
    snow_main_dsc.data = snow_main_buffer + 4;

    // =====================
    // 大猪雪
    // =====================

    uint8_t* snow_big_buffer =
        sdram_malloc(100 * 100 * 3 + 4);

    if(!snow_big_buffer) return;

    read_file_to_array(
        "0:/big_pig_snow.bin",
        snow_big_buffer,
        100 * 100 * 3 + 4
    );

    snow_big_dsc.header.always_zero = 0;
    snow_big_dsc.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    snow_big_dsc.header.w = 100;
    snow_big_dsc.header.h = 100;
    snow_big_dsc.header.reserved = 0;
    snow_big_dsc.data_size = 100 * 100 * 3;
    snow_big_dsc.data = snow_big_buffer + 4;

    // =====================
    // 小猪雪
    // =====================

    uint8_t* snow_small_buffer =
        sdram_malloc(100 * 100 * 3 + 4);

    if(!snow_small_buffer) return;

    read_file_to_array(
        "0:/small_pig_snow.bin",
        snow_small_buffer,
        100 * 100 * 3 + 4
    );

    snow_small_dsc.header.always_zero = 0;
    snow_small_dsc.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    snow_small_dsc.header.w = 100;
    snow_small_dsc.header.h = 100;
    snow_small_dsc.header.reserved = 0;
    snow_small_dsc.data_size = 100 * 100 * 3;
    snow_small_dsc.data = snow_small_buffer + 4;
}

// =========================
// 初始化雪花
// =========================

static void init_snowflakes(void)
{
    for(int i = 0; i < SNOW_COUNT; i++) {

        snowflakes[i].x = rand() % 1024;
        snowflakes[i].y = rand() % 600;

        snowflakes[i].size = 2 + rand() % 4;

        snowflakes[i].speed = 1 + rand() % 2;

        snowflakes[i].drift = -1 + rand() % 3;
    }
}

// =========================
// 飘雪绘制
// =========================

static void snow_draw_cb(lv_event_t *e)
{
    lv_draw_ctx_t *draw_ctx =
        lv_event_get_draw_ctx(e);

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);

    dsc.bg_color = lv_color_white();
    dsc.bg_opa = LV_OPA_80;
    dsc.radius = LV_RADIUS_CIRCLE;

    for(int i = 0; i < SNOW_COUNT; i++) {

        lv_area_t a;

        a.x1 = snowflakes[i].x;
        a.y1 = snowflakes[i].y;

        a.x2 = a.x1 + snowflakes[i].size;
        a.y2 = a.y1 + snowflakes[i].size;

        lv_draw_rect(draw_ctx, &dsc, &a);

        // 更新雪花位置
        snowflakes[i].y += snowflakes[i].speed;
        snowflakes[i].x += snowflakes[i].drift;

        // 出界重置
        if(snowflakes[i].y > 600) {

            snowflakes[i].y = -10;
            snowflakes[i].x = rand() % 1024;
        }

        if(snowflakes[i].x < 0)
            snowflakes[i].x = 1023;

        if(snowflakes[i].x > 1023)
            snowflakes[i].x = 0;
    }
}

// =========================
// 刷新雪
// =========================

static void snow_timer_cb(lv_timer_t *t)
{
    lv_obj_invalidate(lv_scr_act());
}

// =========================
// 检查猪大小变化
// =========================

static void pig_check_cb(lv_timer_t *t)
{
    for(int i = 0; i < 10; i++) {

        const void *src =
            lv_img_get_src(
                pig_fsms[i].pig_t.img_pig
            );

        // 原始大猪 -> 雪天大猪
        if(src ==
           &pig_fsms[0].pig_t.image_pig_big)
        {
            lv_img_set_src(
                pig_fsms[i].pig_t.img_pig,
                &snow_big_dsc
            );
        }

        // 原始小猪 -> 雪天小猪
        else if(src ==
                &pig_fsms[i].pig_t.image_pig_small)
        {
            lv_img_set_src(
                pig_fsms[i].pig_t.img_pig,
                &snow_small_dsc
            );
        }
    }
}

// =========================
// 开启雪天
// =========================

void snow_start(void)
{
    create_snow_image();

    init_snowflakes();

    // 背景
    lv_img_set_src(
        img_main,
        &snow_main_dsc
    );

    // 替换猪
    for(int i = 0; i < 10; i++) {

        if(lv_img_get_src(
            pig_fsms[i].pig_t.img_pig
        ) ==
        &pig_fsms[0].pig_t.image_pig_big)
        {
            lv_img_set_src(
                pig_fsms[i].pig_t.img_pig,
                &snow_big_dsc
            );
        }
        else if(lv_img_get_src(
            pig_fsms[i].pig_t.img_pig
        ) ==
        &pig_fsms[i].pig_t.image_pig_small)
        {
            lv_img_set_src(
                pig_fsms[i].pig_t.img_pig,
                &snow_small_dsc
            );
        }
    }

    // 飘雪绘制
    lv_obj_add_event_cb(
        lv_scr_act(),
        snow_draw_cb,
        LV_EVENT_DRAW_POST,
        NULL
    );

    // 飘雪刷新
    snow_timer = lv_timer_create(
        snow_timer_cb,
        60,
        NULL
    );

    // 检查猪变化
    pig_check_timer = lv_timer_create(
        pig_check_cb,
        500,
        NULL
    );
}

// =========================
// 关闭雪天
// =========================

void snow_stop(void)
{
    // 恢复背景
    lv_img_set_src(
        img_main,
        &image_struct
    );

    // 恢复猪
    for(int i = 0; i < 10; i++) {

        if(lv_img_get_src(
            pig_fsms[i].pig_t.img_pig
        ) ==
        &snow_big_dsc)
        {
            lv_img_set_src(
                pig_fsms[i].pig_t.img_pig,
                &pig_fsms[0].pig_t.image_pig_big
            );
        }
        else if(lv_img_get_src(
            pig_fsms[i].pig_t.img_pig
        ) ==
        &snow_small_dsc)
        {
            lv_img_set_src(
                pig_fsms[i].pig_t.img_pig,
                &pig_fsms[i].pig_t.image_pig_small
            );
        }
    }

    // 删除飘雪回调
    lv_obj_remove_event_cb(
        lv_scr_act(),
        snow_draw_cb
    );

    // 删除雪定时器
    if(snow_timer) {

        lv_timer_del(snow_timer);
        snow_timer = NULL;
    }

    // 删除猪检查定时器
    if(pig_check_timer) {

        lv_timer_del(pig_check_timer);
        pig_check_timer = NULL;
    }

    lv_obj_invalidate(lv_scr_act());
}
