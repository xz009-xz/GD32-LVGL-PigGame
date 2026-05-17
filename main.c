#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

pig_fsm pig_fsms[MAX_PIGS];
Food foods[MAX_FOOD];
float money = 200.0f;
lv_obj_t *coin_label;

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

		ui_game_start(); 
		uint32_t last_tick = lv_tick_get();


		while(1){
			
				delay_us(2000);
				lv_timer_handler();
				uint32_t current_tick = lv_tick_get();
				float dt = (current_tick - last_tick) / 1000.0f;
				if(dt > 0.1f) {
					dt = 0.1f;
				}
				last_tick = current_tick;
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
						else if(pig_fsms[i].growth_fsm.current_state == SLAUGHTER){
								fsm_eventhandle(&pig_fsms[i].growth_fsm, EVENT_GROW);
						}
				}
				if(coin_label != NULL){
						lv_label_set_text_fmt(coin_label, "%.0f", money);
				}
		}
}
