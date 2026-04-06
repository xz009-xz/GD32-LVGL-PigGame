#ifndef GME_TYPE_H
#define GME_TYPE_H

#include <stdbool.h>

#define MAX_PIGS 10
#define TO_BIG_WEIGHT 60
#define TO_SLAUGHTER_WEIGHT 120
#define MAX_FOOD 5

int money;

typedef struct{
    int id;
    int weight;
    int growth;
    int worth;
    bool to_big;
    bool slaughter;
}Pig_t;

typedef struct{
    int cost;
    int growth_boost;
    int weight_boost;
    int num;
    char name[20];
} Food;

//pig×´Ì¬»ú

typedef void (*ActionFun)(void *data);
typedef void (*StateFun)(void *data);
typedef void (*UpdateFun)(void *data,float dt);


typedef enum {
    NORMAL,
    BIG,
    SLAUGHTER
} grow_state;

typedef enum {
    TO_BIG,
    TO_SLAUGHTER,
    TO_NORMAL
} grow_event;

typedef enum {
    ACTION_IDLE,
    ACTION_EAT,
    ACTION_SLEEP
} action_state;

typedef enum 
{
    EVENT_EAT,
    EVENT_SLEEP,
    EVENT_IDLE
} action_event; 

typedef struct{

    grow_state current_state;
    grow_event event;
    ActionFun action;
    grow_state next_state;

}growthFSM;

typedef struct{

    action_state current_state;
    action_event event;
     
    action_state next_state;
}actionFSM;

typedef struct{
    growthFSM growth_fsm;
    actionFSM action_fsm;
    Pig_t pig_t;
} Pig;



#endif