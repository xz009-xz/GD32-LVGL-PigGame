#include "lvgl.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

// ???????????
static uint8_t * bgm_data = NULL;
static uint32_t bgm_size = 22401024;
static uint32_t play_index = 0;


void SPI1_IRQHandler(void)
{
    if(SET == spi_i2s_interrupt_flag_get(SPI1, SPI_I2S_INT_FLAG_TP)) {
        if(bgm_data != NULL) {
            if(play_index >= bgm_size - 1) {
                play_index = 0;
            }

            int16_t sample = bgm_data[play_index] | (bgm_data[play_index + 1] << 8);
            sample >>= 1;
            spi_i2s_data_transmit(SPI1, sample);
            
            play_index += 2;
        } else {
            spi_i2s_data_transmit(SPI1, 0); 
        }
    }
}

void i2s_config(void)
{
    // 1. ????
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_SPI1);
    rcu_spi_clock_config(IDX_SPI1, RCU_SPISRC_PLL0Q);

    // 2. ????
    gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_12 | GPIO_PIN_13);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12 | GPIO_PIN_13);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_12 | GPIO_PIN_13);
    gpio_af_set(GPIOC, GPIO_AF_5, GPIO_PIN_1);
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_1);

    // 3. I2S ?????
    spi_i2s_deinit(SPI1);
    i2s_psc_config(SPI1, I2S_AUDIOSAMPLE_44K, I2S_FRAMEFORMAT_DT16B_CH16B, I2S_MCKOUT_DISABLE);
    i2s_init(SPI1, I2S_MODE_MASTERTX, I2S_STD_PHILIPS, I2S_CKPL_LOW);
    
    // 4. ???????
    i2s_enable(SPI1);
    spi_master_transfer_start(SPI1, SPI_TRANS_START);
    spi_i2s_interrupt_enable(SPI1, SPI_I2S_INT_TP);
}

void music_bgm_load(void)
{
    
    bgm_data = sdram_malloc(bgm_size);
    memset(bgm_data, 0, bgm_size);
    
    if(bgm_data != NULL) {
        
        read_file_to_array("0:/bgm.pcm", bgm_data, bgm_size);
        play_index = 0;
        } 
}
