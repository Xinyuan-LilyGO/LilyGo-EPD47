/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include <esp_heap_caps.h>
#include "esp_log.h"
#include "esp_task_wdt.h"

#include "epd_driver.h"
#ifdef ARDUINO
#include "libjpeg/libjpeg.h"
#else
#include "libjpeg.h"
#endif
#include "ed097oc4.h"
#include "cmd.h"

#define GPIO_MISO 12
#define GPIO_MOSI 13
#define GPIO_SCLK 14
#define GPIO_CS   15

#define RCV_HOST SPI2_HOST

typedef enum te_epd_status
{
    E_EPD_STATUS_RUN = 0,
    E_EPD_STATUS_SLEEP,
    E_EPD_STATUS_MEM_BST_WR,
    E_EPD_STATUS_LD_IMG,
    E_EPD_STATUS_LD_IMG_AREA,
    E_EPD_STATUS_LD_JPEG,
    E_EPD_STATUS_LD_JPEG_AREA,
} te_epd_status;

typedef struct ts_EPDcmd
{
    uint16_t cmd;
    uint16_t len;
    uint8_t data[4096];
} ts_EPDcmd;

/**
 *
 */
typedef struct epd_reg_t
{
    uint32_t address_reg;    // 数据保存的地址
    Rect_t area;
    uint8_t mode;
} epd_reg_t;

uint8_t *data_map;
uint8_t *cur_ptr;
uint32_t jpeg_len;

te_epd_status epd_status = E_EPD_STATUS_SLEEP;

static QueueHandle_t cmd_queue;

epd_reg_t epd_reg;



void unpack_cmd(uint8_t data)
{
    static uint8_t status = 0;
    static ts_EPDcmd cmd;
    static uint8_t *ptr = NULL;

    // printf("unpack_cmd status : %d, data: %d\n", status, data);

    switch (status)
    {
        case 0:
            // printf("head: %d\n", data);
            if (data == 0x55)
            {
                status++;
            }
        break;

        case 1:
            if (data == 0x55)
            {
                status++;
                // printf("start\n");
                ptr = &cmd.data[0];
            }
            else
            {
                status = 0;
            }
        break;

        case 2:
            // printf("data: %d\n", data);
            cmd.cmd = data << 8 & 0xFF00;
            status++;
        break;

        case 3:
            // printf("data: %d\n", data);
            cmd.cmd |= data & 0xFF;
            status++;
        break;

        case 4:
            // printf("data: %d\n", data);
            cmd.len = data << 8 & 0xFF00;
            status++;
        break;

        case 5:
            // printf("data: %d\n", data);
            cmd.len |= data & 0xFF;
            if (cmd.len > 4096)
            {
                status = 0;
                bzero(&cmd, sizeof(cmd));
            }
            else
            {
                status++;
            }
        break;

        case 6:
            if (ptr - &cmd.data[0] < cmd.len)
            {
                *ptr = data;
                ptr++;
            }

            if (ptr - &cmd.data[0] == cmd.len)
            {
                ptr = NULL;
                status = 0;
                printf("%lld cmd: %d, pkg_len: %d\n", esp_timer_get_time(), cmd.cmd, cmd.len);
                xQueueSend(cmd_queue, &cmd, portMAX_DELAY);
                bzero(&cmd, sizeof(cmd));
            }
        break;

        default:
        break;
    }
}


void cmd_process(void *args)
{
    ts_EPDcmd cmd;

    bzero(&cmd, sizeof(cmd));

    for (;;)
    {
        xQueueReceive(cmd_queue, &cmd, portMAX_DELAY);
        switch (epd_status)
        {
            case E_EPD_STATUS_RUN:
                ESP_LOGI("CMD", "Run status");
                if (cmd.cmd == CMD_SLEEP)
                {
                    epd_status = E_EPD_STATUS_SLEEP;
                    epd_poweroff();
                }
                else if (cmd.cmd == CMD_MEM_BST_WR)
                {
                    uint32_t address = (cmd.data[0] << 24) & 0xFFFFFFFF;
                    address |= (cmd.data[1] << 16) & 0xFFFFFFFF;
                    address |= (cmd.data[2] << 8) & 0xFFFFFFFF;
                    address |= cmd.data[3];

                    uint32_t write_len = (cmd.data[4] << 24) & 0xFFFFFFFF;
                    write_len |= (cmd.data[5] << 16) & 0xFFFFFFFF;
                    write_len |= (cmd.data[6] << 8) & 0xFFFFFFFF;
                    write_len |= cmd.data[7];

                    printf("address: %d, write_len: %d\n", address, write_len);
                    memcpy(&data_map[address], &cmd.data[8], write_len);
                }
                else if (cmd.cmd == CMD_MEM_BST_END)
                {
                    // None
                }
                else if (cmd.cmd == CMD_LD_IMG)
                {
                    bzero(&epd_reg, sizeof(epd_reg));
                    // memset(data_map, 0xFF, 540 * 960);
                    epd_reg.mode = cmd.data[0];
                    epd_reg.area.x = 0;
                    epd_reg.area.y = 0;
                    epd_reg.area.width = EPD_WIDTH;
                    epd_reg.area.height = EPD_HEIGHT;
                    epd_status = E_EPD_STATUS_LD_IMG;
                    cur_ptr = &data_map[0];
                }
                else if (cmd.cmd == CMD_LD_IMG_AREA)
                {
                    bzero(&epd_reg, sizeof(epd_reg));
                    // memset(data_map, 0xFF, 540 * 960);
                    epd_reg.mode = cmd.data[0];
                    epd_reg.area.x = cmd.data[1] << 8 & 0xFFFF;
                    epd_reg.area.x |= cmd.data[2] & 0xFFFF;
                    epd_reg.area.y = cmd.data[3] << 8 & 0xFFFF;
                    epd_reg.area.y |= cmd.data[4] & 0xFFFF;
                    epd_reg.area.width = cmd.data[5] << 8 & 0xFFFF;
                    epd_reg.area.width |= cmd.data[6] & 0xFFFF;
                    epd_reg.area.height = cmd.data[7] << 8 & 0xFFFF;
                    epd_reg.area.height |= cmd.data[8] & 0xFFFF;
                    epd_status = E_EPD_STATUS_LD_IMG_AREA;
                    cur_ptr = &data_map[0];
                    printf("recv image - start time: %lld us\n", esp_timer_get_time());
                }
                else if (cmd.cmd == CMD_LD_JPEG)
                {
                    bzero(&epd_reg, sizeof(epd_reg));
                    // memset(data_map, 0xFF, 540 * 960);
                    epd_reg.mode = cmd.data[0];
                    epd_reg.area.x = 0;
                    epd_reg.area.y = 0;
                    epd_reg.area.width = EPD_WIDTH;
                    epd_reg.area.height = EPD_HEIGHT;
                    epd_status = E_EPD_STATUS_LD_JPEG;
                    cur_ptr = &data_map[0];
                    jpeg_len = 0;
                }
                else if (cmd.cmd == CMD_LD_JPEG_AREA)
                {
                    bzero(&epd_reg, sizeof(epd_reg));
                    // memset(data_map, 0xFF, 540 * 960);
                    epd_reg.mode = cmd.data[0];
                    epd_reg.area.x = cmd.data[1] << 8 & 0xFFFF;
                    epd_reg.area.x |= cmd.data[2] & 0xFFFF;
                    epd_reg.area.y = cmd.data[3] << 8 & 0xFFFF;
                    epd_reg.area.y |= cmd.data[4] & 0xFFFF;
                    epd_reg.area.width = cmd.data[5] << 8 & 0xFFFF;
                    epd_reg.area.width |= cmd.data[6] & 0xFFFF;
                    epd_reg.area.height = cmd.data[7] << 8 & 0xFFFF;
                    epd_reg.area.height |= cmd.data[8] & 0xFFFF;
                    epd_status = E_EPD_STATUS_LD_JPEG_AREA;
                    cur_ptr = &data_map[0];
                    jpeg_len = 0;
                }
                else if (cmd.cmd == CMD_CLEAR)
                {
                    epd_reg.area.x = cmd.data[0] << 8 & 0xFFFF;
                    epd_reg.area.x |= cmd.data[1] & 0xFFFF;
                    epd_reg.area.y = cmd.data[2] << 8 & 0xFFFF;
                    epd_reg.area.y |= cmd.data[3] & 0xFFFF;
                    epd_reg.area.width = cmd.data[4] << 8 & 0xFFFF;
                    epd_reg.area.width |= cmd.data[5] & 0xFFFF;
                    epd_reg.area.height = cmd.data[6] << 8 & 0xFFFF;
                    epd_reg.area.height |= cmd.data[7] & 0xFFFF;
                    epd_clear_area(epd_reg.area);
                }
            break;

            case E_EPD_STATUS_SLEEP:
            {
                ESP_LOGI("CMD", "Sleep status");
                if (cmd.cmd == CMD_SYS_RUN)
                {
                    epd_status = E_EPD_STATUS_RUN;
                    epd_poweron();
                    printf("running\n");
                }
            }
            break;

            case E_EPD_STATUS_LD_IMG:
            case E_EPD_STATUS_LD_IMG_AREA:
                ESP_LOGI("CMD", "LD IMG status");
                if (cmd.cmd == CMD_LD_IMG_END)
                {
                    printf("recv image - end time: %lld us\n", esp_timer_get_time());
                    // printf("x: %d, x: %d, width: %d, height: %d\n", epd_reg.area.x, epd_reg.area.y, epd_reg.area.width, epd_reg.area.height);
                    printf("clear - start time: %lld us\n", esp_timer_get_time());
                    epd_clear_area(epd_reg.area);
                    printf("clear - end time: %lld us\n", esp_timer_get_time());
                    printf("draw image - start time: %lld us\n", esp_timer_get_time());
                    epd_draw_grayscale_image(epd_reg.area, &data_map[0]);
                    printf("draw image - end time: %lld us\n", esp_timer_get_time());
                    epd_status = E_EPD_STATUS_RUN;
                }
                else if (cmd.cmd == CMD_MEM_BST_WR)
                {
                    memcpy(cur_ptr, cmd.data, cmd.len);
                    cur_ptr += cmd.len;
                }
            break;

            case E_EPD_STATUS_LD_JPEG:
            case E_EPD_STATUS_LD_JPEG_AREA:
                ESP_LOGI("CMD", "LD JPEG status");
                if (cmd.cmd == CMD_LD_JPEG_END)
                {
                    printf("x: %d, x: %d, width: %d, height: %d\n", epd_reg.area.x, epd_reg.area.y, epd_reg.area.width, epd_reg.area.height);
                    esp_task_wdt_reset();
                    if (epd_status == E_EPD_STATUS_LD_JPEG)
                    {
                        // show_jpg_from_buff(&data_map[epd_reg.address_reg], cur_ptr - &data_map[epd_reg.address_reg]);
                        show_jpg_from_buff(&data_map[epd_reg.address_reg], cur_ptr - &data_map[epd_reg.address_reg], epd_full_screen());
                    }
                    else
                    {
                        printf("show_area_jpg_from_buff\n");
                        show_jpg_from_buff(&data_map[epd_reg.address_reg], cur_ptr - &data_map[epd_reg.address_reg], epd_reg.area);
                    }
                    epd_status = E_EPD_STATUS_RUN;
                }
                else if (cmd.cmd == CMD_MEM_BST_WR)
                {
                    memcpy(cur_ptr, cmd.data, cmd.len);
                    cur_ptr += cmd.len;
                    jpeg_len += cmd.len;
                }
            break;


            default:
            break;
        }
        bzero(&cmd, sizeof(cmd));
    }
}




void IRAM_ATTR spi_slave_trans_done(spi_slave_transaction_t* trans) {
    // printf("[callback] SPI slave transaction finished\n");
    // ((Slave*)trans->user)->results.push_back(trans->trans_len);
    // ((Slave*)trans->user)->transactions.pop_front();
}

WORD_ALIGNED_ATTR char recvbuf[4097] = "";

void main_loop(void)
{
    // esp_err_t ret;
    spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t));

    while (1)
    {
        //Clear receive buffer, set send buffer to something sane
        memset(recvbuf, 0x00, 4096);

        //Set up a transaction of 128 bytes to send/receive
        t.length = 4096 * 8;
        t.tx_buffer = NULL;
        t.rx_buffer = recvbuf;
        /* This call enables the SPI slave interface to send/receive to the sendbuf and recvbuf. The transaction is
        initialized by the SPI master, however, so it will not actually happen until the master starts a hardware transaction
        by pulling CS low and pulsing the clock etc. In this specific example, we use the handshake line, pulled up by the
        .post_setup_cb callback that is called as soon as a transaction is ready, to let the master know it is free to transfer
        data.
        */
        spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);
        printf("%lld recv byte: %d\n", esp_timer_get_time(), t.trans_len / 8);
        for (size_t i = 0; i < (t.trans_len / 8); i++)
        {
            unpack_cmd(recvbuf[i]);
        }
    }
}


void app_main(void)
{
    TaskHandle_t t1;

    data_map = heap_caps_malloc(540 * 960, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    assert(data_map != NULL);
    memset(data_map, 0, 540 * 960);

    //Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .mode = 3,
        .spics_io_num = GPIO_CS,
        .queue_size = 8,
        .flags = 0,
        .post_setup_cb = NULL,
        .post_trans_cb = spi_slave_trans_done
    };

    //Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    cmd_queue = xQueueCreate(8, sizeof(ts_EPDcmd));
    xTaskCreatePinnedToCore((void (*)(void *))cmd_process,
                             "cmd",
                             8000,
                             NULL,
                             10,
                             &t1,
                             1);

    //Initialize SPI slave interface
    ESP_ERROR_CHECK(spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO));

    epd_init();
    libjpeg_init();

    main_loop();
}
