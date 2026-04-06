#include "logic.h"
#include "game_api.h"
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
    ui_pig_to_big(data);
    PigData *pig_data = (PigData *)data;
    printf("Pig %d has grown to big!\n", pig_data->id);
}

void growtoslaughter(void *data){
    ui_pig_to_slaughter(data);
    PigData *pig_data = (PigData *)data;
    printf("Pig %d has been slaughtered!\n", pig_data->id);
}

void growtonormal(void *data){
    ui_pig_to_normal(data);
    PigData *pig_data = (PigData *)data;
    printf("Pig %d has returned to normal!\n", pig_data->id);
}

void on_enter_eat(void *data){
    PigData *pig_data = (PigData *)data;
    printf("Pig %d starts eating!\n", pig_data->id);
    pig_data->eat_timer = 2.0f; //设置吃食物的时间为2秒
}

void on_exit_eat(void *data){
    PigData *pig_data = (PigData *)data;
    printf("Pig %d stops eating!\n", pig_data->id);
    pig_data->eat_timer = 2.0f; //重置吃食物的时间
}

void on_update_eat(void *data,float dt,Food *food){
    PigData *pig_data = (PigData *)data;
    pig_data->hunger -= dt * 10;
    pig_data->growth += dt * food->growth_boost;
    pig_data->weight += dt * food->weight_boost;
    if(pig_data->hunger <= 0){
        pig_data->hunger = 0;
        printf("Pig %d is full!\n", pig_data->id);
    }
    if(pig_data->eat_timer < 0){
        printf("Pig %d has finished eating!\n", pig_data->id);
    }
    
}

void on_update_idle(void *data,float dt){
    PigData *pig_data = (PigData *)data;
    pig_data->hunger += dt * 5; //空闲状态下饥饿度增加
    if(pig_data->hunger >= 100){
        pig_data->hunger = 100;
        printf("Pig %d is very hungry!\n", pig_data->id);
    }
}

//行为状态机的状态表

