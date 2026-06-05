#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"
#include "gamelogic/disaster.h"
#include <stdlib.h>
#include "gamelogic/auth.h"
#include "gamelogic/save.h"

pig_fsm pig_fsms[MAX_PIGS];
Food foods[MAX_FOOD];
float money = 200.0f;
lv_obj_t *coin_label;
char current_user[MAX_USERNAME_LEN + 1] = {0};
float slaughter_number = 0.0f;
lv_obj_t *slaughter_label;

float rand_float(void) {
    return (float)rand() / (float)RAND_MAX;
}

int main()
{
		sys_init();
		
		rcu_periph_clock_enable(RCU_GPIOA);
		gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
		gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
		gpio_bit_reset(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
		
		lv_init();
		lv_port_disp_init();
		lv_port_indev_init();

		//bgm
		i2s_config();
		nvic_irq_enable(SPI1_IRQn, 0, 0);
		music_bgm_load();
	
		ui_game_start(); 
		uint32_t last_tick = lv_tick_get();
		disaster_init();

		while(1){
			
				delay_us(2000);
				lv_timer_handler();
				uint32_t current_tick = lv_tick_get();
				float dt = (current_tick - last_tick) / 1000.0f;
				if(dt > 0.1f) {
					dt = 0.1f;
				}
				last_tick = current_tick;
				disaster_update(dt);
				static float trigger_timer = 0.0f;
				static float chance = 0.0f;          
				trigger_timer += dt;
				if (trigger_timer >= 30.0f) {
					trigger_timer = 0.0f;
					if (!disaster_is_active()) {
						
						float avg_hunger = 0.0f, avg_weight = 0.0f;
						for (int i = 0; i < MAX_PIGS; i++) {
							avg_hunger += pig_fsms[i].pig_t.hunger;
							avg_weight += pig_fsms[i].pig_t.weight;
						}
						avg_hunger /= MAX_PIGS;
						avg_weight /= MAX_PIGS;
	
						   
						if (avg_hunger > 60) chance += 0.5f;
						if (avg_weight > 80) chance += 0.01f;
						if (chance > 1.0f) chance = 1.0f;
	
						if (rand_float() < chance) {
							DisasterType disaster_type;
							switch (rand() % 3) {
								case 0:  disaster_type = DISASTER_HOT;  break;
								case 1:  disaster_type = DISASTER_SNOW; break;
								default: disaster_type = DISASTER_RAIN; break;
							}
							disaster_start(disaster_type, 20.0f);
							chance = 0.0f;    
						}
					}
				}
				for(int i = 0; i < MAX_PIGS; i++){
						fsm_update(&pig_fsms[i].action_fsm, dt);
				}
				for(int i = 0; i < MAX_PIGS; i++){
		
						if(pig_fsms[i].action_fsm.current_state == ACTION_EAT && pig_fsms[i].pig_t.eat_timer <= 0){
								fsm_eventhandle(&pig_fsms[i].action_fsm, EVENT_STOP_EAT);
						}
				}
				for(int i = 0; i < MAX_PIGS; i++){
						if(pig_fsms[i].pig_t.growth >= 50 && pig_fsms[i].growth_fsm.current_state == NORMAL){
								fsm_eventhandle(&pig_fsms[i].growth_fsm, EVENT_GROW);
						}
						else if(pig_fsms[i].pig_t.growth >= 100 && pig_fsms[i].growth_fsm.current_state == BIG){
								fsm_eventhandle(&pig_fsms[i].growth_fsm, EVENT_GROW);
						}
				}
				if(coin_label != NULL){
						lv_label_set_text_fmt(coin_label, "%.0f", money);
				}
				if(slaughter_label != NULL){
						lv_label_set_text_fmt(slaughter_label, "%.0f", slaughter_number);
				}
				for(int i = 0; i < MAX_PIGS; i++){
						if(pig_fsms[i].pig_t.slaughter_icon != NULL && pig_fsms[i].pig_t.is_slaughtering == false){
								slaughter_number += 1.0f;
								pig_fsms[i].pig_t.is_slaughtering = true;
								break;
						}
				}
		}
}

	
