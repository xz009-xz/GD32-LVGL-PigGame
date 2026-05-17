#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

static lv_obj_t *food = NULL;

void fruit_cb(lv_event_t *e)
{
    void *user_data = lv_event_get_user_data(e);
    int i = (intptr_t)user_data;
    lv_event_code_t code = lv_event_get_code(e);

    lv_indev_t *indev = lv_indev_get_act();
    lv_point_t point;
    lv_indev_get_point(indev, &point);

    //按下生成图片
    if(code == LV_EVENT_PRESSED)
    {
        food = lv_img_create(lv_scr_act());
        lv_img_set_src(food, &foods[i].img);
        lv_obj_set_pos(food, point.x - 30, point.y - 30);
        lv_obj_move_foreground(food);
        //lv_obj_add_flag(food, LV_OBJ_FLAG_CLICKABLE);
    }
    if(code == LV_EVENT_PRESSING)
    {
        lv_obj_set_pos(food, point.x - 30, point.y - 30);
    }
    if(code == LV_EVENT_RELEASED)
    {
        // 松手时：先删图
        int x = lv_obj_get_x(food);
        int y = lv_obj_get_y(food);
        lv_obj_del(food);
        food = NULL;

        for(int j=0; j<10; j++){
            int pig_x = pig_fsms[j].pig_t.x;
            int pig_y = pig_fsms[j].pig_t.y;
            int pig_w = 100;
            int pig_h = 100;

            if (x >= pig_x && x <= pig_x + pig_w &&
                y >= pig_y && y <= pig_y + pig_h && money >= foods[i].cost) {
             
                money -= foods[i].cost;  
                pig_feed_anim(j, i);
               
                pig_fsms[j].pig_t.eat_fruit_idx = i;
         
                fsm_eventhandle(&pig_fsms[j].action_fsm, EVENT_START_EAT);
                
                return;
            }
        }
    }
}
