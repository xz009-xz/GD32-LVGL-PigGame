#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "logic.h"

lv_img_dsc_t image_info1,image_info2;
lv_obj_t *info_img;
lv_obj_t *info_cover;
static int t = 0;

//创建信息框的图片
void create_info_image(void)
{
    if(t==1){
        return;
    }

    uint8_t* image_buffer = sdram_malloc( 400 * 300 * 3 + 4 );
    read_file_to_array("0:/info.bin", image_buffer,  400 * 300 * 3 + 4 );
    image_info1.header.always_zero = 0;
    image_info1.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    image_info1.header.w = 400;
    image_info1.header.h = 300;
    image_info1.header.reserved = 0;
    image_info1.data_size = 400 * 300 * 3;
    image_info1.data = image_buffer + 4; 

    image_buffer = sdram_malloc( 400 * 300 * 3 + 4 );
    read_file_to_array("0:/info.bin", image_buffer,  400 * 300 * 3 + 4 );

    int w = 400;
    int h = 300;
    uint8_t* src = image_buffer + 4;   // 原始像素
    uint8_t* dst = sdram_malloc(w * h * 3); // 翻转后的缓存

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int src_idx = (y * w + x) * 3;
            int dst_x = w - 1 - x;        // 水平翻转
            int dst_idx = (y * w + dst_x) * 3;

            dst[dst_idx + 0] = src[src_idx + 0];
            dst[dst_idx + 1] = src[src_idx + 1];
            dst[dst_idx + 2] = src[src_idx + 2];
        }
    }

    image_info2.header.always_zero = 0;
    image_info2.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    image_info2.header.w = 400;
    image_info2.header.h = 300;
    image_info2.header.reserved = 0;
    image_info2.data_size = 400 * 300 * 3;
    image_info2.data = dst;

    t=1;
}



//创建全屏遮罩，点击遮罩关闭信息框
void cover_event_cb(lv_event_t *e)
{
    lv_obj_del(info_img);
    lv_obj_del(info_cover);
    for(int j=0; j<10; j++){
        //lv_obj_clear_flag(pig_fsms[j].pig_t.img_pig, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(pig_fsms[j].pig_t.img_pig, LV_OPA_100, 0);
    }
}

void create_screen_cover(void)
{
    info_cover = lv_btn_create(lv_scr_act());
    lv_obj_set_size(info_cover, 1024, 600);
    lv_obj_set_pos(info_cover, 0, 0);
    lv_obj_move_foreground(info_cover);
	lv_obj_set_style_opa(info_cover, LV_OPA_0, 0);
    lv_obj_add_event_cb(info_cover, cover_event_cb, LV_EVENT_PRESSED, NULL);
}

void ui_info(lv_event_t *e)
{
    int i = (intptr_t)lv_event_get_user_data(e);

    for(int j=0; j<10; j++){
        if (j != i) {
            //lv_obj_add_flag(pig_fsms[j].pig_t.img_pig, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_opa(pig_fsms[j].pig_t.img_pig, LV_OPA_50, 0);
        } 
    }

    create_info_image();

    create_screen_cover();

    int pig_x = pig_fsms[i].pig_t.x;
    int pig_y = pig_fsms[i].pig_t.y;
    info_img = lv_img_create(lv_scr_act());

    lv_obj_t * label_id = lv_label_create(info_img);
    lv_obj_t * label_growth = lv_label_create(info_img);
    lv_obj_t * label_weight = lv_label_create(info_img);
    lv_obj_t * label_hunger = lv_label_create(info_img);
    lv_label_set_text_fmt(label_id, "ID: %d", pig_fsms[i].pig_t.id);
    lv_label_set_text_fmt(label_growth, "Growth: %d", pig_fsms[i].pig_t.growth);
    lv_label_set_text_fmt(label_weight, "Weight: %d", pig_fsms[i].pig_t.weight);
    lv_label_set_text_fmt(label_hunger, "Hunger: %d", pig_fsms[i].pig_t.hunger);
    lv_obj_set_style_text_font(label_id, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_font(label_growth, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_font(label_weight, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_font(label_hunger, &lv_font_montserrat_24, 0);

    //012567为info1，3489为info2
    if(i == 0 || i == 1 || i == 2 || i == 5 || i == 6 || i == 7){
        lv_img_set_src(info_img, &image_info1);
        lv_obj_set_pos(info_img, pig_x+120, pig_y-100);

        lv_obj_set_pos(label_id, 185, 35);
        lv_obj_set_pos(label_growth, 145, 90);
        lv_obj_set_pos(label_weight, 145, 137);
        lv_obj_set_pos(label_hunger, 145, 189);
    }
    else {
        lv_img_set_src(info_img, &image_info2);
        lv_obj_set_pos(info_img, pig_x-420, pig_y-100);

        lv_obj_set_pos(label_id, 160, 35);
        lv_obj_set_pos(label_growth, 120, 90);
        lv_obj_set_pos(label_weight, 120, 137);
        lv_obj_set_pos(label_hunger, 120, 189);
    }
    lv_obj_move_foreground(info_img);
    lv_obj_add_flag(info_img, LV_OBJ_FLAG_CLICKABLE);
}
