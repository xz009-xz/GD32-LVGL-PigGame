#ifndef LOGIC_H
#define LOGIC_H

#include "common/game_type.h"
#include "common/game_api.h"

#define MAX_STATES 10

void create_food(Food *foods);
//int *money_init(void);
void free_food(Food *food);


 
typedef enum
{
    EVENT_NONE = 0,
    EVENT_GROW,
    EVENT_START_EAT,
    EVENT_STOP_EAT

} Event;

typedef void (*ActionFun)(void *data);
typedef void (*StateFun)(void *data);
typedef void (*UpdateFun)(void *data,float dt);

typedef struct {

    Event event;
    int current_state;
    ActionFun action;
    int next_state;

}Fsm_table;

typedef struct {
      
    int current_state;
    Fsm_table *table;
    int table_size;
    StateFun enterFun[MAX_STATES];
    StateFun exitFun[MAX_STATES];
    UpdateFun updateFun[MAX_STATES];
    void *data;

} Fsm;

void fsm_init(Fsm *fsm, Fsm_table *table, int table_size, int initial_state);
void enter_init(Fsm *fsm, int state, StateFun enter_fun);
void exit_init(Fsm *fsm, int state, StateFun exit_fun);
void update_init(Fsm *fsm, int state, UpdateFun update_fun);
void fsm_statetransform(Fsm *fsm, int new_state);
void fsm_eventhandle(Fsm *fsm, Event event);
void fsm_update(Fsm *fsm, float dt);


typedef enum{
    NORMAL,
    BIG,
    SLAUGHTER

} grow_state;

typedef enum{
    ACTION_IDLE,
    ACTION_EAT,

} action_state;

typedef struct{
    int id;
    float growth;
    float weight;
    float hunger;
    float eat_timer;
    int eat_fruit_idx;     
    int x,y;
    lv_img_dsc_t image_pig_small;
    lv_img_dsc_t image_pig_big;
    lv_obj_t *img_pig;
    lv_obj_t *slaughter_icon;
    bool is_slaughtering;  // SLAUGHTER 状态下的红色感叹图标

} PigData;

typedef struct{
    Fsm growth_fsm;
    Fsm action_fsm;
    PigData pig_t;
} pig_fsm;

void growtobig(void *data);
void growtoslaughter(void *data);
void growtonormal(void *data);
void on_enter_eat(void *data);
void on_exit_eat(void *data);
void on_enter_slaughter(void *data);
void on_exit_slaughter(void *data);
void on_update_eat(void *data,float dt);
void on_update_idle(void *data,float dt);


void pig_fsm_init(pig_fsm *pig_fsm_instance,int id);

extern pig_fsm pig_fsms[MAX_PIGS];
extern Food foods[MAX_FOOD];
extern float money;
extern lv_obj_t *coin_label;
extern lv_obj_t *slaughter_label;
extern float slaughter_number;

#endif
