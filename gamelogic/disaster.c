#include<stdio.h>
#include<stdbool.h>
#include "disaster.h"
#include "game_api.h"

typedef struct {

    bool active;
    DisasterType type;
    float timer;
    float duration;

}DisasterManager;

typedef struct {

    DisasterType type;
    float weight_loss_rate;
    float hunger_increase_rate;
    float growth_rate_modifier;
    float eat_effect_modifier;

}DisasterConfig;

static DisasterManager manager;
static DisasterConfig configs[DISASTER_MAX] = {
    { DISASTER_NONE, 0.0f, 1.0f, 1.0f, 1.0f },
    { DISASTER_HOT, -0.01f, 0.02f, 0.9f, 0.8f },
    { DISASTER_SNOW, -0.02f, 0.03f, 0.8f, 0.7f },
    { DISASTER_RAIN, -0.015f, 0.025f, 0.85f, 0.75f }
};

void disaster_init(void){

    manager.active = false;
    manager.type = DISASTER_NONE;
    manager.timer = 0.0f;
    manager.duration = 0.0f;

}

void disaster_start(DisasterType type,float duration){

    if(type <= DISASTER_NONE || type >= DISASTER_MAX){
        return;
    }

    if(manager.active){
        switch(manager.type){
            case DISASTER_HOT:
                hot_stop();
                break;
            case DISASTER_SNOW:
                snow_stop();
                break;
            case DISASTER_RAIN:
                rain_stop();
                break;
            default:
                break;
        }
        manager.active = false;
        manager.type = DISASTER_NONE;
        manager.timer = 0.0f;
        manager.duration = 0.0f;
    }

    manager.active = true;
    manager.type = type;
    manager.timer = 0.0f;
    manager.duration = duration;
    switch(type){
        case DISASTER_HOT:
            hot_start();
            break;
        case DISASTER_SNOW:
            snow_start();
            break;
        case DISASTER_RAIN:
            rain_start();
            break;
        default:
            break;
    }

}

void disaster_stop(void){

    if(!manager.active){
        return;
    }

    switch(manager.type){
        case DISASTER_HOT:
            hot_stop();
            break;
        case DISASTER_SNOW:
            snow_stop();
            break;
        case DISASTER_RAIN:
            rain_stop();
            break;
        default:
            break;
    }
    manager.active = false;
    manager.type = DISASTER_NONE;
    manager.timer = 0.0f;
    manager.duration = 0.0f;

}

void disaster_update(float dt){

    if(!manager.active){
        return;
    }

    manager.timer += dt;
    if(manager.timer >= manager.duration){
        disaster_stop();
    }
}

bool disaster_is_active(void){
    return manager.active;
}

DisasterType disaster_get_type(void){
    return manager.type;
}

DisasterModifier disaster_get_modifier(void){
    if(!manager.active){
        DisasterModifier mod = {1.0f, 1.0f, 1.0f, 1.0f};
        return mod;
    }
    DisasterConfig config = configs[manager.type];
    DisasterModifier mod = {
        .weight_loss_rate = config.weight_loss_rate,
        .hunger_increase_rate = config.hunger_increase_rate,
        .growth_rate_modifier = config.growth_rate_modifier,
        .eat_effect_modifier = config.eat_effect_modifier
    };
    return mod;
}

float disaster_get_remaining_time(void){
    if(!manager.active){
        return 0.0f;
    }
    //��һ��;
    return (manager.duration - manager.timer) / manager.duration;
}
