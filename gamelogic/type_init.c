#include "game_type.h"
 
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>

Pig *create_pig(void){
    Pig *pig = (Pig *)malloc(sizeof(Pig) * MAX_PIGS);
    if (pig == NULL) {
        fprintf(stderr, "Memory allocation failed for pigs.\n");
        return NULL;
    }
    for (int i = 0; i < MAX_PIGS; i++) {
        pig[i].id = i + 1;
        pig[i].weight = 5;
        pig[i].growth = 0;
        pig[i].worth = 0;
        pig[i].to_big = false;
        pig[i].slaughter = false;
    }
    return pig;
}

Food *create_food(void){
    Food *food = (Food *)malloc(sizeof(Food) * MAX_FOOD);
    if (food == NULL) {
        fprintf(stderr, "Memory allocation failed for food.\n");
        return NULL;
    }
    food[0].cost = 5;
    food[0].growth_boost = 2;
    food[0].weight_boost = 3;
    food[0].num = 0;
    strcpy(food[0].name, "Corn");
    food[1].cost = 10;
    food[1].growth_boost = 4;
    food[1].weight_boost = 5;
    food[1].num = 0;
    strcpy(food[1].name, "Hay");
    food[2].cost = 15;
    food[2].growth_boost = 6;
    food[2].weight_boost = 8;
    food[2].num = 0;
    strcpy(food[2].name, "Grain");
    food[3].cost = 20;
    food[3].growth_boost = 8;
    food[3].weight_boost = 10;
    food[3].num = 0;
    strcpy(food[3].name, "Silage");
    food[4].cost = 22;
    food[4].growth_boost = 10;
    food[4].weight_boost = 12;
    food[4].num = 0;
    strcpy(food[4].name, "Feed");
    return food;
}

int *money_init(void){
    int *money = (int *)malloc(sizeof(int));
    if (money == NULL) {
        fprintf(stderr, "Memory allocation failed for money.\n");
        return NULL;
    }
    *money = 100;
    return money;
}

void free_pig(Pig *pig){
    if (pig != NULL) {
        free(pig);
    }
}

void free_food(Food *food){
    if (food != NULL) {
        free(food);
    }
}
