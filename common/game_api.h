#ifndef __GAME_API_H
#define __GAME_API_H

#include "lvgl.h"

void ui_game_start(void);
void ui_game_screen(lv_event_t *e);
void pig_grow_anim(int pig_idx);
void pig_small_anim(int pig_idx);
void pig_slaughter_anim(int pig_idx);
void fruit_cb(lv_event_t *e);
void pig_feed_anim(int pig_idx, int food_idx);
void ui_info(lv_event_t *e);
void rain_start(void);
void rain_stop(void);
void hot_start(void);
void hot_stop(void);

#endif
