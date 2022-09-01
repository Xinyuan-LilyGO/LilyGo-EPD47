
/******************************************************************************/
/***        include files                                                   ***/
/******************************************************************************/

#include "rmt_pulse.h"

#include <driver/rmt.h>

/******************************************************************************/
/***        macro definitions                                               ***/
/******************************************************************************/

/******************************************************************************/
/***        type definitions                                                ***/
/******************************************************************************/

/******************************************************************************/
/***        local function prototypes                                       ***/
/******************************************************************************/

/******************************************************************************/
/***        exported variables                                              ***/
/******************************************************************************/

/******************************************************************************/
/***        local variables                                                 ***/
/******************************************************************************/

/**
 * @brief the RMT channel configuration object
 */
static rmt_config_t row_rmt_config;

/******************************************************************************/
/***        exported functions                                              ***/
/******************************************************************************/

void rmt_pulse_init(gpio_num_t pin)
{
    row_rmt_config.rmt_mode = RMT_MODE_TX;
    // currently hardcoded: use channel 0
    row_rmt_config.channel = RMT_CHANNEL_1;

    row_rmt_config.gpio_num = pin;
    row_rmt_config.mem_block_num = 2;

    // Divide 80MHz APB Clock by 8 -> .1us resolution delay
    row_rmt_config.clk_div = 8;

    row_rmt_config.tx_config.loop_en = false;
    row_rmt_config.tx_config.carrier_en = false;
    row_rmt_config.tx_config.carrier_level = RMT_CARRIER_LEVEL_LOW;
    row_rmt_config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
    row_rmt_config.tx_config.idle_output_en = true;

    rmt_config(&row_rmt_config);
    rmt_driver_install(RMT_CHANNEL_1, 0, 0);
}


void IRAM_ATTR pulse_ckv_ticks(uint16_t high_time_ticks,
                               uint16_t low_time_ticks, bool wait)
{
    rmt_item32_t rmt_mem_ptr;
    if (high_time_ticks > 0)
    {
        rmt_mem_ptr.level0 = 1;
        rmt_mem_ptr.duration0 = high_time_ticks;
        rmt_mem_ptr.level1 = 0;
        rmt_mem_ptr.duration1 = low_time_ticks;
    }
    else
    {
        rmt_mem_ptr.level0 = 1;
        rmt_mem_ptr.duration0 = low_time_ticks;
        rmt_mem_ptr.level1 = 0;
        rmt_mem_ptr.duration1 = 0;
    }

    rmt_write_items(row_rmt_config.channel, &rmt_mem_ptr, 1, wait);
}


void IRAM_ATTR pulse_ckv_us(uint16_t high_time_us, uint16_t low_time_us, bool wait)
{
    rmt_item32_t rmt_mem_ptr;
    if (high_time_us * 10 > 0)
    {
        rmt_mem_ptr.level0 = 1;
        rmt_mem_ptr.duration0 = high_time_us * 10;
        rmt_mem_ptr.level1 = 0;
        rmt_mem_ptr.duration1 = low_time_us * 10;
    }
    else
    {
        rmt_mem_ptr.level0 = 1;
        rmt_mem_ptr.duration0 = low_time_us * 10;
        rmt_mem_ptr.level1 = 0;
        rmt_mem_ptr.duration1 = 0;
    }

    rmt_write_items(row_rmt_config.channel, &rmt_mem_ptr, 1, wait);
}

/******************************************************************************/
/***        local functions                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/