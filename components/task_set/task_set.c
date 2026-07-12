#include <stdio.h>
#include "task_set.h"

app_set_t app = {
    .uart = {
        .tx_data = "",
        .rx_data = {0},
        .handle = NULL
    },
    .setting = {
        .screen_light = 100,
        .wifi_status = 0,
        .wifi_scan = true,
        .wifi_ssid = {'\0'},
        .wifi_password = NULL,
        .wifi_networks = NULL,
        .wifi_connect = false,
        .wifi_disconnect = false,
        .wifi_failure = 0,
        .handle = NULL
    },
    .data = {
        .time_init = false,
        .weather_update = false,
        .handle = NULL
    },
    .music = {
        .music_name = {'\0'},
        .music_list = NULL,
        .music_play = false,
        .music_pause = false,
        .music_start = false,
        .music_mode = 0,
        .music_index = 0,
        .music_volume = 100,
        .handle = NULL
    },
    .xiaozhi = {
        .xiaozhi_start = false,
        .xiaozhi_status = 0,
    }
};

void task_init(void)
{
    // 设置 APP
    task_create(&app.setting.handle, wifi_task, "wifi_task", 1024 * 6);
    // 串口 APP
    task_create(&app.uart.handle, uart_task, "uart_task", 1024 * 5);
    // 日历和天气 APP
    task_create(&app.data.handle, data_task, "data_task", 1024 * 6);
    // 音乐 APP
    task_create(&app.music.handle, music_task, "music_task", 1024 * 6);
    // 小智 APP
    task_create(&app.xiaozhi.handle, xiaozhi_task, "xiaozhi_task", 1024 * 8);
}

void task_create(TaskHandle_t *handle, TaskFunction_t func, const char *name, uint32_t stack)
{
    if (*handle == NULL) {
        xTaskCreatePinnedToCore(func, name, stack, NULL, 1, handle, 0);
    }
}

void task_delete(TaskHandle_t *handle)
{
    if (*handle != NULL) {
        vTaskDelete(*handle);
        *handle = NULL;
    }
}