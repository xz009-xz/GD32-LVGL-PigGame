#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

#define MAX_RAINDROPS 60
#define RAIN_MIN_LEN 20
#define RAIN_MAX_LEN 50
#define RAIN_MIN_SPEED 5
#define RAIN_MAX_SPEED 12
#define RAIN_MIN_WIDTH 3
#define RAIN_MAX_WIDTH 6

typedef struct {
    int x, y;
    int length;
    int speed;
    int width;
} raindrop_t;

static raindrop_t raindrops[MAX_RAINDROPS];
static lv_timer_t *rain_timer = NULL;
static bool rain_enabled = false;

// ?????
static void init_raindrops(void) {
    for(int i = 0; i < MAX_RAINDROPS; i++) {
        raindrops[i].x = rand() % LV_HOR_RES;
        raindrops[i].y = rand() % LV_VER_RES;
        raindrops[i].length = RAIN_MIN_LEN + rand() % (RAIN_MAX_LEN - RAIN_MIN_LEN + 1);
        raindrops[i].speed = RAIN_MIN_SPEED + rand() % (RAIN_MAX_SPEED - RAIN_MIN_SPEED + 1);
        raindrops[i].width = RAIN_MIN_WIDTH + rand() % (RAIN_MAX_WIDTH - RAIN_MIN_WIDTH + 1);
    }
}

// ???? + ?????
static void rain_draw_post_cb(lv_event_t *e) {
    if(!rain_enabled) return;

    lv_obj_t *scr = lv_event_get_target(e);
    lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);

    // ????????
    lv_draw_rect_dsc_t fog_dsc;
    lv_draw_rect_dsc_init(&fog_dsc);
    fog_dsc.bg_color = lv_color_hex(0x555555); // ????
    fog_dsc.bg_opa = LV_OPA_60;                // ?????,?????
    lv_area_t fog_area = {0, 0, LV_HOR_RES - 1, LV_VER_RES - 1};
    lv_draw_rect(draw_ctx, &fog_dsc, &fog_area);

    // ????
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = lv_color_white();
    rect_dsc.bg_opa = LV_OPA_90;

    for(int i = 0; i < MAX_RAINDROPS; i++) {
        lv_area_t area;
        area.x1 = raindrops[i].x;
        area.y1 = raindrops[i].y;
        area.x2 = raindrops[i].x + raindrops[i].width - 1;
        area.y2 = raindrops[i].y + raindrops[i].length;
        lv_draw_rect(draw_ctx, &rect_dsc, &area);

        raindrops[i].y += raindrops[i].speed;
        if(raindrops[i].y > LV_VER_RES) raindrops[i].y = -raindrops[i].length;
    }
}

// ?????
static void rain_timer_cb(lv_timer_t *timer) {
    lv_obj_invalidate(lv_scr_act());
}

// ??
void rain_start(void) {
    if(rain_enabled) return;

    rain_enabled = true;
    init_raindrops();

    lv_obj_add_event_cb(lv_scr_act(), rain_draw_post_cb, LV_EVENT_DRAW_POST, NULL);

    if(!rain_timer)
        rain_timer = lv_timer_create(rain_timer_cb, 30, NULL);
}

// ??
void rain_stop(void) {
    if(!rain_enabled) return;

    rain_enabled = false;

    lv_obj_remove_event_cb(lv_scr_act(), rain_draw_post_cb);

    if(rain_timer) {
        lv_timer_del(rain_timer);
        rain_timer = NULL;
    }

    lv_obj_invalidate(lv_scr_act());
}