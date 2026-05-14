#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

// ==============================
// 水果拖动 + 松手删除 事件回调
// ==============================
static void drag_food_event_cb(lv_event_t *e)
{
    lv_obj_t *img = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    // 获取水果编号
    int fruit_idx = (intptr_t)lv_obj_get_user_data(img);

    // 拖动时跟着手指走
    if (code == LV_EVENT_PRESSING) {
        lv_indev_t *indev = lv_indev_get_act();
        lv_point_t pos;
        lv_indev_get_point(indev, &pos);
        lv_obj_set_pos(img, pos.x - 30, pos.y - 30);
        return;
    }

    // 松手时：你要的逻辑 —— 先删图！！！
    if (code == LV_EVENT_RELEASED)
    {
        // 先获取坐标
        int x = lv_obj_get_x(img);
        int y = lv_obj_get_y(img);

        // 你要求：先删图片！！！
        lv_obj_del(img);

        // 再判断猪
        for(int i=0; i<MAX_PIGS; i++){
            int pig_x = pig_fsms[i].pig_t.x;
            int pig_y = pig_fsms[i].pig_t.y;
            int pig_w = 100;
            int pig_h = 100;

            if (x >= pig_x && x <= pig_x + pig_w &&
                y >= pig_y && y <= pig_y + pig_h)
            {
                pig_feed_anim(i, fruit_idx);
                return;
            }
        }
    }
}

void fruit_cb(lv_event_t *e)
{
    void *user_data = lv_event_get_user_data(e);
    int i = (intptr_t)user_data;

    lv_indev_t *indev = lv_indev_get_act();
    lv_point_t point;
    lv_indev_get_point(indev, &point);

    lv_obj_t *food = lv_img_create(lv_scr_act());
    lv_img_set_src(food, &foods[i].img);
    lv_obj_set_pos(food, point.x - 30, point.y - 30);
    lv_obj_move_foreground(food);
		lv_obj_add_flag(food, LV_OBJ_FLAG_CLICKABLE);
	
		lv_event_send(food, LV_EVENT_PRESSING, NULL);

    lv_obj_set_user_data(food, (void*)(intptr_t)i);
    lv_obj_add_event_cb(food, drag_food_event_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(food, drag_food_event_cb, LV_EVENT_RELEASED, NULL);
}
