#include "game_type.h"
#include "logic.h"
#include "drivers.h"

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>



void create_food(Food *foods){
    foods[0].cost = 5;
    foods[0].growth_boost = 2;
    foods[0].weight_boost = 3;
    foods[0].num = 0;
    strcpy(foods[0].name, "Corn");
    uint8_t* image_buffer = sdram_malloc( 60 * 71 * 3 + 4 );
	read_file_to_array("0:/fruit1.bin", image_buffer,  60 * 71 * 3 + 4 );
	foods[0].img.header.always_zero = 0;
	foods[0].img.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	foods[0].img.header.w = 60;
	foods[0].img.header.h = 71;
	foods[0].img.header.reserved = 0;
	foods[0].img.data_size = 60 * 71 * 3;
	foods[0].img.data = image_buffer + 4;
    image_buffer = sdram_malloc( 66 * 63 * 3 + 4 );
	read_file_to_array("0:/fruit2.bin", image_buffer,  66 * 63 * 3 + 4 );
	foods[1].img.header.always_zero = 0;
	foods[1].img.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	foods[1].img.header.w = 66;
	foods[1].img.header.h = 63;
	foods[1].img.header.reserved = 0;
	foods[1].img.data_size = 66 * 63 * 3;
	foods[1].img.data = image_buffer + 4;
    foods[1].cost = 10;
    foods[1].growth_boost = 4;
    foods[1].weight_boost = 5;
    foods[1].num = 0;
    strcpy(foods[1].name, "Hay");
    image_buffer = sdram_malloc( 68 * 73 * 3 + 4 );
	read_file_to_array("0:/fruit3.bin", image_buffer,  68 * 73 * 3 + 4 );
	foods[2].img.header.always_zero = 0;
	foods[2].img.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	foods[2].img.header.w = 68;
	foods[2].img.header.h = 73;
	foods[2].img.header.reserved = 0;
	foods[2].img.data_size = 68 * 73 * 3;
	foods[2].img.data = image_buffer + 4;
    foods[2].cost = 15;
    foods[2].growth_boost = 6;
    foods[2].weight_boost = 8;
    foods[2].num = 0;
    strcpy(foods[2].name, "Grain");
}
/*
int *money_init(void){
    int *money = (int *)malloc(sizeof(int));
    if (money == NULL) {
        fprintf(stderr, "Memory allocation failed for money.\n");
        return NULL;
    }
    *money = 100;
    return money;
}*/

void free_food(Food *food){
    if (food != NULL) {
        free(food);
    }
}

// pig_init() ??????? - ?? ui_game_screen() ??????????? pig_fsms[] ?????

