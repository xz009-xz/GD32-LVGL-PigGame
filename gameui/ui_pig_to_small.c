#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

int pig_idx, x, y;
lv_img_dsc_t sell_image1,cage_image;
lv_obj_t *sell_img,*cage_img;

#define ANIM_TIME 800

void img_move(void* var, int32_t v)
{
    lv_obj_t *img = (lv_obj_t *)var;
    lv_obj_set_y(img, v);
}

void img_del(struct _lv_anim_t *a)
{
    lv_obj_t *img = (lv_obj_t *)a->var;
    lv_obj_del(img);
}

//图片透明度逐渐变大，最后删除图片
void img_hiding(void* var, int32_t v)
{
    lv_obj_t *img = (lv_obj_t *)var;
    lv_obj_set_style_opa(img, v, 0);
}

void img_hiden(struct _lv_anim_t *a)
{
    lv_anim_t anim1;
    lv_anim_init(&anim1);
    lv_anim_set_var(&anim1, cage_img);
    lv_anim_set_exec_cb(&anim1, img_hiding);
    lv_anim_set_values(&anim1, 100, 0);
    lv_anim_set_time(&anim1, ANIM_TIME);
    lv_anim_set_repeat_count(&anim1, 0);
    lv_anim_set_ready_cb(&anim1, img_del);
    lv_anim_start(&anim1);
}

void new_pig(struct _lv_anim_t *a)
{
    //换成笼子图片
    cage_img = lv_img_create(lv_scr_act());
	lv_img_set_src(cage_img, &cage_image);
    lv_obj_set_pos(cage_img, x, -100);

    lv_img_set_src(pig_fsms[pig_idx].pig_t.img_pig, &pig_fsms[pig_idx].pig_t.image_pig_small);
    
    lv_anim_t anim1;
    lv_anim_init(&anim1);
    lv_anim_set_var(&anim1, cage_img);
    lv_anim_set_exec_cb(&anim1, img_move);
    lv_anim_set_values(&anim1, -100, y);
    lv_anim_set_time(&anim1, ANIM_TIME);
    lv_anim_set_repeat_count(&anim1, 0);
    lv_anim_set_ready_cb(&anim1, img_hiden);
    lv_anim_set_delay(&anim1, 1000);
    lv_anim_start(&anim1);
    
    lv_anim_t anim2;
    lv_anim_init(&anim2);
    lv_anim_set_var(&anim2, pig_fsms[pig_idx].pig_t.img_pig);
    lv_anim_set_exec_cb(&anim2, img_move);
    lv_anim_set_values(&anim2, -100, y);
    lv_anim_set_time(&anim2, ANIM_TIME);
    lv_anim_set_repeat_count(&anim2, 0);
    lv_anim_set_delay(&anim2, 1000);
    lv_anim_start(&anim2);
}

void drop_finish_cb(struct _lv_anim_t *a)
{
    lv_anim_t anim1;
    lv_anim_init(&anim1);
    lv_anim_set_var(&anim1, sell_img);
    lv_anim_set_exec_cb(&anim1, img_move);
    lv_anim_set_values(&anim1, y - 380, -480); // ??????
    lv_anim_set_time(&anim1, ANIM_TIME);
    lv_anim_set_repeat_count(&anim1, 0);
    lv_anim_set_ready_cb(&anim1, img_del);
    lv_anim_start(&anim1);
    
    lv_anim_t anim2;
    lv_anim_init(&anim2);
    lv_anim_set_var(&anim2, pig_fsms[pig_idx].pig_t.img_pig);
    lv_anim_set_exec_cb(&anim2, img_move);
    lv_anim_set_values(&anim2, y, -100); // ??
    lv_anim_set_time(&anim2, ANIM_TIME);
    lv_anim_set_repeat_count(&anim2, 0);
    lv_anim_set_ready_cb(&anim2, new_pig);
    lv_anim_start(&anim2);
}

void pig_small_anim(int i)
{
	pig_idx=i;

    // ????
    uint8_t* image_buffer = sdram_malloc( 120 * 480 * 3 + 4 );
	read_file_to_array("0:/sell.bin", image_buffer,  120 * 480 * 3 + 4 );
	sell_image1.header.always_zero = 0;
	sell_image1.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	sell_image1.header.w = 120;
	sell_image1.header.h = 480;
	sell_image1.header.reserved = 0;
	sell_image1.data_size = 120 * 480 * 3;
	sell_image1.data = image_buffer + 4;
	sell_img = lv_img_create(lv_scr_act());
	lv_img_set_src(sell_img, &sell_image1);
    x = pig_fsms[pig_idx].pig_t.x;
    y = pig_fsms[pig_idx].pig_t.y;
    lv_obj_set_pos(sell_img, x-10, -480);

    //cage
    image_buffer = sdram_malloc( 100 * 100 * 3 + 4 );
	read_file_to_array("0:/cage.bin", image_buffer,  100 * 100 * 3 + 4 );
	cage_image.header.always_zero = 0;
	cage_image.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	cage_image.header.w = 100;
	cage_image.header.h = 100;
	cage_image.header.reserved = 0;
	cage_image.data_size = 100 * 100 * 3;
	cage_image.data = image_buffer + 4;

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, sell_img);
    lv_anim_set_exec_cb(&anim, img_move);
    lv_anim_set_values(&anim, -480, y - 380); // ?????
    lv_anim_set_time(&anim, ANIM_TIME);
    lv_anim_set_repeat_count(&anim, 0);
    lv_anim_set_delay(&anim, 1000);
    lv_anim_set_ready_cb(&anim, drop_finish_cb);
    lv_anim_start(&anim);
}

