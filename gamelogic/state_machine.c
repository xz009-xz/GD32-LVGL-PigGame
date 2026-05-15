#include "logic.h"
#include "game_api.h"
#include "drivers.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
 
//状态机对象函数
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
//pig实例函数
void growtobig(void *data){
    PigData *pig_data = (PigData *)data;
    pig_grow_anim(pig_data->id);
    //printf("Pig %d has grown to big!\n", pig_data->id);
}

void growtoslaughter(void *data){
    PigData *pig_data = (PigData *)data;
    //pig_slaughter_anim(pig_data->id);
    //printf("Pig %d has been slaughtered!\n", pig_data->id);
}

void growtonormal(void *data){
    PigData *pig_data = (PigData *)data;
    //pig_slaughter_anim(pig_data->id);
    //printf("Pig %d has returned to normal!\n", pig_data->id);
}

void on_enter_eat(void *data){
    PigData *pig_data = (PigData *)data;
    //printf("Pig %d starts eating!\n", pig_data->id);
    pig_data->eat_timer = 2.0f; //设置吃食物的时间为2秒
}

void on_exit_eat(void *data){
    PigData *pig_data = (PigData *)data;
    //printf("Pig %d stops eating!\n", pig_data->id);
    pig_data->eat_timer = 2.0f; //重置吃食物的时间
}

void on_update_eat(void *data,float dt,Food *food){
    PigData *pig_data = (PigData *)data;
    pig_data->hunger -= dt * 10;
    /*pig_data->growth += dt * food->growth_boost;
    pig_data->weight += dt * food->weight_boost;
    */
   //先这样改，之后换回来
    pig_data->growth += dt * food[0].growth_boost;
    pig_data->weight += dt * food[0].weight_boost;
    if(pig_data->hunger <= 0){
        pig_data->hunger = 0;
        //printf("Pig %d is full!\n", pig_data->id);
    }
    if(pig_data->eat_timer < 0){
        //printf("Pig %d has finished eating!\n", pig_data->id);
    }
    
}

void on_update_idle(void *data,float dt){
    PigData *pig_data = (PigData *)data;
    pig_data->hunger += dt * 5; //空闲状态下饥饿度增加
    if(pig_data->hunger >= 100){
        pig_data->hunger = 100;
        //printf("Pig %d is very hungry!\n", pig_data->id);
    }
}

//行为状态机的状态表

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

//pig的init
void pig_fsm_init(pig_fsm *pig_fsm_instance,int id){
    pig_fsm_instance->pig_t.id = id;
    pig_fsm_instance->pig_t.growth = rand() % 40; 
    pig_fsm_instance->pig_t.weight = rand() % 30;
    pig_fsm_instance->pig_t.hunger = rand() % 80;
    pig_fsm_instance->pig_t.eat_timer = 0;
    
    uint8_t* image_buffer = sdram_malloc( 100 * 100 * 3 + 4 );
	read_file_to_array("0:/ui_pig_small.bin", image_buffer,  100 * 100 * 3 + 4 );
	pig_fsm_instance->pig_t.image_pig_small.header.always_zero = 0;
	pig_fsm_instance->pig_t.image_pig_small.header.cf = LV_IMG_CF_TRUE_COLOR;
	pig_fsm_instance->pig_t.image_pig_small.header.w = 100;
	pig_fsm_instance->pig_t.image_pig_small.header.h = 100;
	pig_fsm_instance->pig_t.image_pig_small.header.reserved = 0;
	pig_fsm_instance->pig_t.image_pig_small.data_size = 100 * 100 * 3;
	pig_fsm_instance->pig_t.image_pig_small.data = image_buffer + 4; 

    image_buffer = sdram_malloc( 100 * 100 * 3 + 4 );
	read_file_to_array("0:/ui_pig_big.bin", image_buffer,  100 * 100 * 3 + 4 );
	pig_fsm_instance->pig_t.image_pig_big.header.always_zero = 0;
	pig_fsm_instance->pig_t.image_pig_big.header.cf = LV_IMG_CF_TRUE_COLOR;
	pig_fsm_instance->pig_t.image_pig_big.header.w = 100;
	pig_fsm_instance->pig_t.image_pig_big.header.h = 100;
	pig_fsm_instance->pig_t.image_pig_big.header.reserved = 0;
	pig_fsm_instance->pig_t.image_pig_big.data_size = 100 * 100 * 3;
	pig_fsm_instance->pig_t.image_pig_big.data = image_buffer + 4; 

    fsm_init(&pig_fsm_instance->growth_fsm, state_table, state_table_size, NORMAL);
    fsm_init(&pig_fsm_instance->action_fsm, action_table, action_table_size, ACTION_IDLE);

    enter_init(&pig_fsm_instance->action_fsm, ACTION_EAT, on_enter_eat);
    exit_init(&pig_fsm_instance->action_fsm, ACTION_EAT, on_exit_eat);
    //update_init(&pig_fsm_instance->action_fsm, ACTION_EAT, on_update_eat);
    update_init(&pig_fsm_instance->action_fsm, ACTION_IDLE, on_update_idle);
}




