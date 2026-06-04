#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

lv_img_dsc_t image_hunder[2];
lv_obj_t *img_hunger[10];

void create_hunger_img(){
    uint8_t* image_buffer = sdram_malloc( 30 * 30 * 3 + 4 );
	read_file_to_array("0:/hunger1.bin", image_buffer,  30 * 30 * 3 + 4 );
	image_hunder[0].header.always_zero = 0;
	image_hunder[0].header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	image_hunder[0].header.w = 30;
	image_hunder[0].header.h = 30;
	image_hunder[0].header.reserved = 0;
	image_hunder[0].data_size = 30 * 30 * 3;
	image_hunder[0].data = image_buffer + 4;

    image_buffer = sdram_malloc( 30 * 30 * 3 + 4 );
    read_file_to_array("0:/hunger2.bin", image_buffer,  30 * 30 * 3 + 4 );
	image_hunder[1].header.always_zero = 0;
	image_hunder[1].header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	image_hunder[1].header.w = 30;
	image_hunder[1].header.h = 30;
	image_hunder[1].header.reserved = 0;
	image_hunder[1].data_size = 30 * 30 * 3;
	image_hunder[1].data = image_buffer + 4;
}

void show_hunger(int pig_idx,int hunger_level){

    pig_idx = 0,hunger_level = 1;//测试
    if(!img_hunger[pig_idx]){
        img_hunger[pig_idx] = lv_img_create(pig_fsms[pig_idx].pig_t.img_pig);
        lv_obj_set_pos(img_hunger[pig_idx], 70, 70);
        lv_img_set_src(img_hunger[pig_idx], &image_hunder[hunger_level]);
    }
}

void hide_hunger(int pig_idx){

    pig_idx = 0;//测试

    if(img_hunger[pig_idx]){
        lv_obj_del(img_hunger[pig_idx]);
        img_hunger[pig_idx] = NULL;
    }
}