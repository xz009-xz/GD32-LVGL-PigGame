#ifndef LOGIC_H
#define LOGIC_H

#include "common/game_type.h"
#include "common/game_api.h"

#define MAX_STATES 10

//逻辑数据创建函数
Pig *create_pig(void);
Food *create_food(void);
int *money_init(void);
void free_pig(Pig *pig);
void free_food(Food *food);

//状态机对象
 
typedef enum
{
    EVENT_NONE = 0,
    EVENT_GROW,
    EVENT_START_EAT,
    EVENT_STOP_EAT

} Event;

typedef (*ActionFun)(void *data);
typedef (*StateFun)(void *data);
typedef (*UpdateFun)(void *data,float dt);

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

//pig状态机实例
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
    int growth;
    int weight;
    int hunger;
    float eat_timer;
    
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
void on_update_eat(void *data,float dt,Food *food);
void on_update_idle(void *data,float dt);

#endif 