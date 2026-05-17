#ifndef GME_TYPE_H
#define GME_TYPE_H

#include <stdbool.h>
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"

#define MAX_PIGS 10
#define TO_BIG_WEIGHT 60
#define TO_SLAUGHTER_WEIGHT 120
#define MAX_FOOD 3

extern float money;

typedef struct{
    int cost;
    int growth_boost;
    int weight_boost;
    int num;
    char name[20];
    lv_img_dsc_t img;
} Food;




#endif
