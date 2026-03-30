
/******************************************************************************/
/***        include files                                                   ***/
/******************************************************************************/

#include "rmt_pulse.h"

#include <esp_idf_version.h>
#if ESP_IDF_VERSION_MAJOR >= 5 // IDF 5+ — new RMT TX driver (driver_ng)
#include <driver/rmt_tx.h>
#include <driver/rmt_encoder.h>
#include <freertos/FreeRTOS.h>  // portMAX_DELAY
#else
#include <driver/rmt.h>
#endif

#include <esp_log.h>

/******************************************************************************/
/***        local variables                                                 ***/
/******************************************************************************/

#if ESP_IDF_VERSION_MAJOR >= 5

static rmt_channel_handle_t rmt_chan = NULL;
static rmt_encoder_handle_t copy_encoder = NULL;

#else

/**
 * @brief the RMT channel configuration object
 */
static rmt_config_t row_rmt_config;

#endif

/******************************************************************************/
/***        exported functions                                              ***/
/******************************************************************************/

void rmt_pulse_init(gpio_num_t pin)
{
#if ESP_IDF_VERSION_MAJOR >= 5
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = pin,
        .mem_block_symbols = 64,
        // 10 MHz -> 0.1 us resolution (same as legacy clk_div=8 on 80MHz APB)
        .resolution_hz = 10 * 1000 * 1000,
        .trans_queue_depth = 4,
        .flags.invert_out = false,
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &rmt_chan));

    rmt_copy_encoder_config_t copy_encoder_config = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_encoder_config, &copy_encoder));

    ESP_ERROR_CHECK(rmt_enable(rmt_chan));
#else
    row_rmt_config.rmt_mode = RMT_MODE_TX;
    // currently hardcoded: use channel 1
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
#endif
}


void IRAM_ATTR pulse_ckv_ticks(uint16_t high_time_ticks,
                               uint16_t low_time_ticks, bool wait)
{
#if ESP_IDF_VERSION_MAJOR >= 5
    rmt_symbol_word_t symbol;
    if (high_time_ticks > 0) {
        symbol.level0    = 1;
        symbol.duration0 = high_time_ticks;
        symbol.level1    = 0;
        symbol.duration1 = low_time_ticks > 0 ? low_time_ticks : 1;
    } else {
        symbol.level0    = 1;
        symbol.duration0 = low_time_ticks;
        symbol.level1    = 0;
        symbol.duration1 = 1;
    }

    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };
    rmt_transmit(rmt_chan, copy_encoder, &symbol, sizeof(symbol), &tx_config);
    if (wait) {
        rmt_tx_wait_all_done(rmt_chan, portMAX_DELAY);
    }
#else
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
#endif
}


void IRAM_ATTR pulse_ckv_us(uint16_t high_time_us, uint16_t low_time_us, bool wait)
{
    pulse_ckv_ticks(10 * high_time_us, 10 * low_time_us, wait);
}

/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/
