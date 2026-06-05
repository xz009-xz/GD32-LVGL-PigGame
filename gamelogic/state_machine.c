#include "logic.h"
#include "game_api.h"
#include "drivers.h"
#include "disaster.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
 
void fsm_init(Fsm *fsm, Fsm_table *table,int table_size,int initial_state){
    fsm->current_state = initial_state;
    fsm->table = table;
    fsm->table_size = table_size;
    memset(fsm->enterFun, 0, sizeof(fsm->enterFun));
    memset(fsm->exitFun, 0, sizeof(fsm->exitFun));
    memset(fsm->updateFun, 0, sizeof(fsm->updateFun));
}

void enter_init(Fsm *fsm,int state,StateFun enter_fun){
    if(state < MAX_STATES && state >= 0 ){
        fsm->enterFun[state] = enter_fun;
    }
}

void exit_init(Fsm *fsm,int state,StateFun exit_fun){
    if(state < MAX_STATES && state >= 0 ){
        fsm->exitFun[state] = exit_fun;
    }
}

void update_init(Fsm *fsm,int state,UpdateFun update_fun){
    if(state < MAX_STATES && state >= 0 ){
        fsm->updateFun[state] = update_fun;
    }
}

void fsm_statetransform(Fsm *fsm,int new_state){
 if(fsm ->current_state == new_state){
    return;
 }
 if(fsm->exitFun[fsm->current_state] != NULL){
     fsm->exitFun[fsm->current_state](fsm->data);
 }
 fsm ->current_state = new_state;
 if(fsm->enterFun[fsm->current_state] != NULL){
     fsm->enterFun[fsm->current_state](fsm->data);
 }
}

void fsm_eventhandle(Fsm *fsm,Event event){

    for (int i = 0; i < fsm->table_size;++i){

        const Fsm_table *rule = &fsm->table[i];
        if(rule->current_state == fsm->current_state && rule->event == event){
            if(rule->action != NULL){
                rule->action(fsm->data);
            }
            fsm_statetransform(fsm,rule->next_state);
            break;
        }
    }
}

void fsm_update(Fsm *fsm,float dt){
    if(fsm->updateFun[fsm->current_state] != NULL){
        fsm->updateFun[fsm->current_state](fsm->data,dt);
    }
}

void growtobig(void *data){
    PigData *pig_data = (PigData *)data;
    pig_grow_anim(pig_data->id);
    //printf("Pig %d has grown to big!\n", pig_data->id);
}

void growtoslaughter(void *data){
    PigData *pig_data = (PigData *)data;
    // 金币在用户点击感叹号时结�?
    (void)pig_data;
    //printf("Pig %d has been slaughtered!\n", pig_data->id);
}

void growtonormal(void *data){
    PigData *pig_data = (PigData *)data;
    pig_small_anim(pig_data->id);
    pig_data->growth = (float)(rand() % 40);
    pig_data->weight = (float)(rand() % 30);
    pig_data->hunger = (float)(rand() % 80);
    pig_data->eat_timer = 0;
    pig_data->eat_fruit_idx = -1;
    //printf("Pig %d has returned to normal!\n", pig_data->id);
}

void on_enter_eat(void *data){
    PigData *pig_data = (PigData *)data;
    pig_data->eat_timer = 2.0f; 
}

void on_exit_eat(void *data){
    PigData *pig_data = (PigData *)data;
    pig_data->eat_timer = 0;
    pig_data->eat_fruit_idx = -1; 
}

void on_update_eat(void *data,float dt){
    extern Food foods[MAX_FOOD];
    PigData *pig_data = (PigData *)data;
    DisasterModifier mod = disaster_get_modifier();

    pig_data->eat_timer -= dt;
    if(pig_data->eat_timer <= 0){
        pig_data->eat_timer = 0;
        return;
    }
    
    // ??? eat_fruit_idx ????????????
    int idx = pig_data->eat_fruit_idx;
    if(idx < 0 || idx >= MAX_FOOD) return;
    
    pig_data->hunger -= dt * 10 * mod.eat_effect_modifier;
    if(pig_data->hunger < 0){
        pig_data->hunger = 0;
    }
    
    pig_data->growth += dt * foods[idx].growth_boost * mod.growth_rate_modifier;
    if(pig_data->growth > 100){
        pig_data->growth = 100;
    }
    pig_data->weight += dt * foods[idx].weight_boost * mod.weight_loss_rate;
}

void on_update_idle(void *data,float dt){
    PigData *pig_data = (PigData *)data;
    DisasterModifier mod = disaster_get_modifier();
    pig_data->hunger += dt * 0.5f*mod.hunger_increase_rate; 
    if(pig_data->hunger >= 100){
        pig_data->hunger = 100;
        pig_data->growth -= dt * 0.5f*mod.growth_rate_modifier;
        if(pig_data->growth < 0){
            pig_data->growth = 0;
        }
        pig_data->weight -= dt * 0.5f*mod.weight_loss_rate;
        if(pig_data->weight < 0){
            pig_data->weight = 0;
        }
        //printf("Pig %d is very hungry!\n", pig_data->id);
    }
}

void slaughter_icon_cb(lv_event_t *e)
{
    int pig_idx = (int)(uintptr_t)lv_event_get_user_data(e);
    money += pig_fsms[pig_idx].pig_t.weight;                     // 金币结算
    fsm_eventhandle(&pig_fsms[pig_idx].growth_fsm, EVENT_GROW);  // 触发 pig_small_anim + 新猪初始�? + 切换�? NORMAL
}

void on_enter_slaughter(void *data)
{
    PigData *pig_data = (PigData *)data;
    int pig_idx = pig_data->id;

    if(pig_data->slaughter_icon != NULL) return;

    lv_obj_t *parent = lv_obj_get_parent(pig_data->img_pig);

    pig_data->slaughter_icon = lv_btn_create(parent);
    lv_obj_set_size(pig_data->slaughter_icon, 30, 30);
    lv_obj_set_pos(pig_data->slaughter_icon,
                   pig_data->x + 85,
                   pig_data->y + 85);
    lv_obj_set_style_bg_color(pig_data->slaughter_icon, lv_color_hex(0xFF0000), 0);
    lv_obj_t *label = lv_label_create(pig_data->slaughter_icon);
    lv_label_set_text(label, "!");
    lv_obj_center(label);
    lv_obj_add_event_cb(pig_data->slaughter_icon, slaughter_icon_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)pig_idx);

    lv_obj_move_foreground(pig_data->slaughter_icon);
}

void on_exit_slaughter(void *data){
    PigData *pig_data = (PigData *)data;
    if(pig_data->slaughter_icon != NULL){
        lv_obj_del(pig_data->slaughter_icon);
        slaughter_number -= 1.0f;
        pig_data->slaughter_icon = NULL;
    }
}

static Fsm_table state_table[] = {
    {EVENT_GROW, NORMAL, growtobig, BIG},
    {EVENT_GROW, BIG, growtoslaughter, SLAUGHTER},
    {EVENT_GROW, SLAUGHTER, growtonormal, NORMAL}
};
static const int state_table_size = sizeof(state_table) / sizeof(state_table[0]);

static Fsm_table action_table[] = {
    {EVENT_START_EAT, ACTION_IDLE, NULL, ACTION_EAT},
    {EVENT_STOP_EAT, ACTION_EAT, NULL, ACTION_IDLE}
};
static const int action_table_size = sizeof(action_table) / sizeof(action_table[0]);


void pig_fsm_init(pig_fsm *pig_fsm_instance,int id){
    pig_fsm_instance->pig_t.id = id;
    pig_fsm_instance->pig_t.growth = (float)(rand() % 40);
    pig_fsm_instance->pig_t.weight = (float)(rand() % 30);
    pig_fsm_instance->pig_t.hunger = (float)(rand() % 80);
    pig_fsm_instance->pig_t.eat_timer = 0;
    pig_fsm_instance->pig_t.eat_fruit_idx = -1;
    pig_fsm_instance->pig_t.slaughter_icon = NULL;
    
    uint8_t* image_buffer = sdram_malloc( 100 * 100 * 3 + 4 );
	read_file_to_array("0:/ui_pig_small.bin", image_buffer,  100 * 100 * 3 + 4 );
	pig_fsm_instance->pig_t.image_pig_small.header.always_zero = 0;
	pig_fsm_instance->pig_t.image_pig_small.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	pig_fsm_instance->pig_t.image_pig_small.header.w = 100;
	pig_fsm_instance->pig_t.image_pig_small.header.h = 100;
	pig_fsm_instance->pig_t.image_pig_small.header.reserved = 0;
	pig_fsm_instance->pig_t.image_pig_small.data_size = 100 * 100 * 3;
	pig_fsm_instance->pig_t.image_pig_small.data = image_buffer + 4; 

    image_buffer = sdram_malloc( 100 * 100 * 3 + 4 );
	read_file_to_array("0:/ui_pig_big.bin", image_buffer,  100 * 100 * 3 + 4 );
	pig_fsm_instance->pig_t.image_pig_big.header.always_zero = 0;
	pig_fsm_instance->pig_t.image_pig_big.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	pig_fsm_instance->pig_t.image_pig_big.header.w = 100;
	pig_fsm_instance->pig_t.image_pig_big.header.h = 100;
	pig_fsm_instance->pig_t.image_pig_big.header.reserved = 0;
	pig_fsm_instance->pig_t.image_pig_big.data_size = 100 * 100 * 3;
	pig_fsm_instance->pig_t.image_pig_big.data = image_buffer + 4; 

    fsm_init(&pig_fsm_instance->growth_fsm, state_table, state_table_size, NORMAL);
    fsm_init(&pig_fsm_instance->action_fsm, action_table, action_table_size, ACTION_IDLE);

    pig_fsm_instance->growth_fsm.data = &pig_fsm_instance->pig_t;
    pig_fsm_instance->action_fsm.data = &pig_fsm_instance->pig_t;

    enter_init(&pig_fsm_instance->action_fsm, ACTION_EAT, on_enter_eat);
    exit_init(&pig_fsm_instance->action_fsm, ACTION_EAT, on_exit_eat);
    exit_init(&pig_fsm_instance->growth_fsm, SLAUGHTER, on_exit_slaughter);
    enter_init(&pig_fsm_instance->growth_fsm, SLAUGHTER, on_enter_slaughter);
    update_init(&pig_fsm_instance->action_fsm, ACTION_EAT, on_update_eat);
    update_init(&pig_fsm_instance->action_fsm, ACTION_IDLE, on_update_idle);
}




