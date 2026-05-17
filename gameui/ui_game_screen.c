#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

lv_img_dsc_t image_struct;
lv_img_dsc_t image_fruit[3];

//	pig shack animation
static void pig_shake_anim_callback(void* var, int32_t v)
{
    int idx = (int)(uintptr_t)var;
    lv_obj_set_x(pig_fsms[idx].pig_t.img_pig, pig_fsms[idx].pig_t.x + v);
}

// 抖动函数（现在只需要猪的索引）
void pig_shack_anim(int pig_idx)
{
    //lv_obj_t *pig_img = pig_fsms[pig_idx].pig_t.img_pig;

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, (void*)(uintptr_t)pig_idx);
    lv_anim_set_exec_cb(&anim, pig_shake_anim_callback);

    lv_anim_set_values(&anim, -2, 2);
    lv_anim_set_time(&anim, 400);
    lv_anim_set_playback_time(&anim, 400);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);

    lv_anim_start(&anim);
}

void ui_game_screen(lv_event_t *e)
{
	lv_obj_t *game_screen = lv_obj_create(NULL);
	lv_obj_clear_flag(game_screen, LV_OBJ_FLAG_SCROLLABLE);
	// Create game screen
	uint8_t* image_buffer = sdram_malloc( 1024 * 600 * 3 + 4 );
	read_file_to_array("0:/main.bin", image_buffer,  1024 * 600 * 3 + 4 );
	image_struct.header.always_zero = 0;
	image_struct.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	image_struct.header.w = 1024;
	image_struct.header.h = 600;
	image_struct.header.reserved = 0;
	image_struct.data_size = 1024 * 600 * 3;
	image_struct.data = image_buffer + 4;
	lv_obj_t *img_main = lv_img_create(game_screen);
	lv_img_set_src(img_main, &image_struct);

    //pig - 使用 pig_fsm_init 初始化
    for(int i = 0; i < MAX_PIGS; i++){
        pig_fsm_init(&pig_fsms[i], i);
        pig_fsms[i].pig_t.x = 170 + i % 5 * 120 + 50;
        pig_fsms[i].pig_t.y = (i > 4) ? 350 : 200;
        pig_fsms[i].pig_t.img_pig = lv_img_create(game_screen);
        lv_img_set_src(pig_fsms[i].pig_t.img_pig, &pig_fsms[i].pig_t.image_pig_small);
        lv_obj_set_pos(pig_fsms[i].pig_t.img_pig, pig_fsms[i].pig_t.x, pig_fsms[i].pig_t.y);
        lv_obj_add_flag(pig_fsms[i].pig_t.img_pig, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_add_event_cb(pig_fsms[i].pig_t.img_pig, ui_info, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
		pig_shack_anim(i);
    }

	   // 初始化食物数据
	create_food(foods);
	lv_obj_t *btn_fruit[3];
	for(int i=0;i<3;i++){
		btn_fruit[i] = lv_btn_create(game_screen);
		lv_obj_set_size(btn_fruit[i], 110, 100);
		lv_obj_align(btn_fruit[i], LV_ALIGN_TOP_LEFT, 328+i*128, 497);
		lv_obj_add_event_cb(btn_fruit[i],fruit_cb, LV_EVENT_ALL, (void*)i);
		lv_obj_set_style_opa(btn_fruit[i], LV_OPA_0, 0);
	}
	coin_label = lv_label_create(game_screen);
	lv_label_set_text_fmt(coin_label, "%.0f", money);
	lv_obj_align(coin_label, LV_ALIGN_TOP_LEFT, 110, 46);
	lv_obj_set_style_text_color(coin_label, lv_color_hex(0xFF9933), 0);
	lv_obj_set_style_text_font(coin_label, &lv_font_montserrat_26, 0);

	//fruit
	/*
	lv_obj_t *btn_fruit[3];
	for(int i=0;i<3;i++){
		btn_fruit[i] = lv_btn_create(game_screen);
		lv_obj_set_size(btn_fruit[i], 110, 100);
		lv_obj_align(btn_fruit[i], LV_ALIGN_TOP_LEFT, 328+i*128, 497);
		lv_obj_add_event_cb(btn_fruit[i],pig_feed_anim, LV_EVENT_PRESSED, (void*)i);
	}*/

	//测试代码，后续会删除
	/*
	lv_obj_t *btn_back1 = lv_btn_create(game_screen);
	lv_obj_set_size(btn_back1, 80, 40);
	lv_obj_align(btn_back1, LV_ALIGN_TOP_LEFT, 10, 10);
	lv_obj_add_event_cb(btn_back1,pig_grow_anim, LV_EVENT_RELEASED, NULL);

	lv_obj_t *btn_back2 = lv_btn_create(game_screen);
	lv_obj_set_size(btn_back2, 80, 40);
	lv_obj_align(btn_back2, LV_ALIGN_TOP_LEFT, 100, 10);
	lv_obj_add_event_cb(btn_back2,pig_small_anim, LV_EVENT_RELEASED, NULL);

	lv_obj_t *btn_back3 = lv_btn_create(game_screen);
	lv_obj_set_size(btn_back3, 80, 40);
	lv_obj_align(btn_back3, LV_ALIGN_TOP_LEFT, 190, 10);
	lv_obj_add_event_cb(btn_back3,pig_feed_anim, LV_EVENT_RELEASED, NULL);
*/
    lv_scr_load_anim(
        game_screen,
        LV_SCR_LOAD_ANIM_FADE_ON,
        300,
        0,
        true
    );
}
