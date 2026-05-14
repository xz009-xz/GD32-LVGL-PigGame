#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"

lv_img_dsc_t image_struct1;

void ui_game_start(void)
{
	lv_obj_t *start_screen=lv_obj_create(NULL);

	uint8_t* image_buffer = sdram_malloc( 1024 * 600 * 3 + 4 );
	read_file_to_array("0:/img_start.bin", image_buffer,  1024 * 600 * 3 + 4 );
	image_struct1.header.always_zero = 0;
	image_struct1.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	image_struct1.header.w = 1024;
	image_struct1.header.h = 600;
	image_struct1.header.reserved = 0;
	image_struct1.data_size = 1024 * 600 * 3;
	image_struct1.data = image_buffer + 4; 
	lv_obj_t *img_start = lv_img_create(start_screen);
	lv_img_set_src(img_start, &image_struct1);

    // Create start button
    lv_obj_t *start_btn = lv_btn_create(start_screen);
    lv_obj_set_size(start_btn, 450, 110);
    lv_obj_align(start_btn, LV_ALIGN_CENTER, 0, 180);
    lv_obj_add_event_cb(start_btn, ui_game_screen, LV_EVENT_RELEASED, NULL);

    lv_obj_set_style_bg_opa(start_btn, LV_OPA_TRANSP, 0);  
    lv_obj_set_style_border_opa(start_btn, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_shadow_opa(start_btn, LV_OPA_TRANSP, 0); 
		
	lv_scr_load(start_screen);
}
