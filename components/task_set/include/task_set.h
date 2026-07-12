#ifndef TASK_SET_H
#define TASK_SET_H

#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lv_adapter.h"
#include "ui.h"
#include "uart_set.h"
#include "lcd_set.h"
#include "wifi_set.h"
#include "data_set.h"
#include "music_set.h"
#include "xiaozhi_set.h"
#include "lv_games.h"

// UART application state
typedef struct {
    const char *tx_data;
    char rx_data[256];
    TaskHandle_t handle;        // 串口任务句柄 (NULL=未创建)
} uart_set_t;

typedef struct {
    uint8_t screen_light;       // 屏幕亮度设置
    uint8_t wifi_status;        // WiFi 状态
    bool wifi_scan;             // 是否进行 WiFi 扫描
    char wifi_ssid[64];         // WiFi 名称
    char * wifi_password;       // WiFi 密码
    char * wifi_networks;       // 扫描到的 WiFi 列表
    bool wifi_connect;          // 连接 WiFi 标志位
    bool wifi_disconnect;       // 断开 WiFi 标志位
    uint8_t wifi_failure;       // WiFi 连接失败次数
    TaskHandle_t handle;        // WiFi任务句柄 (NULL=未创建)
} setting_set_t;

typedef struct {
    bool time_init;             // 时间初始化标志位
    bool weather_update;        // 天气刷新标志位
    TaskHandle_t handle;        // Data任务句柄 (NULL=未创建)
} data_set_t;

typedef struct {
    char music_name[128];       // 当前歌曲名
    char *music_list;           // 歌曲名拼接列表
    bool music_play;            // 音乐播放标志位
    bool music_pause;           // 音乐暂停标志位
    bool music_start;           // 音乐开始播放标志位
    uint8_t music_mode;         // 音乐播放模式 (0=单曲循环, 1=顺序播放, 2=随机播放)
    uint16_t music_index;       // 当前歌曲索引
    uint8_t music_volume;       // 音乐音量 (0~100)
    TaskHandle_t handle;        // 音乐任务句柄
} music_set_t;

typedef struct {
    bool xiaozhi_start;         // 小智启动标志位
    uint8_t xiaozhi_status;     // 小智状态 (0=未启动, 1=启动中, 2=关闭中, 3=对话中)
    TaskHandle_t handle;        // 小智任务句柄
} xiaozhi_set_t;

// Global application state (will be extended in future chapters)
typedef struct {
    uart_set_t uart;
    setting_set_t setting;   
    data_set_t data;
    music_set_t music;       
    xiaozhi_set_t xiaozhi;
} app_set_t;

extern app_set_t app;

/**
 * @brief 初始化任务管理系统
 *
 * 初始化全局应用状态 {@link app}，为后续任务创建做好准备。
 * 应在 app_main() 中优先调用。
 */
void task_init(void);

/**
 * @brief 创建一个 FreeRTOS 任务
 *
 * 封装 xTaskCreate()，简化任务创建流程。
 * 任务创建成功后，其句柄会写入 @p handle 指向的变量。
 *
 * @param[out] handle 任务句柄指针，创建成功后保存任务句柄
 * @param[in]  func   任务入口函数
 * @param[in]  name   任务名称（用于调试，最大长度由 FreeRTOS configMAX_TASK_NAME_LEN 决定）
 * @param[in]  stack  任务栈大小（单位：字，即 uint32_t）
 *
 * @note 若任务创建失败，会通过 ESP_LOGE 输出错误日志
 */
void task_create(TaskHandle_t *handle, TaskFunction_t func, const char *name, uint32_t stack);

/**
 * @brief 删除一个 FreeRTOS 任务
 *
 * 安全删除指定任务。若任务句柄有效，则调用 vTaskDelete() 删除任务
 * 并将句柄置为 NULL。
 *
 * @param[in,out] handle 任务句柄指针，删除后会被置为 NULL
 *
 * @note 传入 NULL 指针或 *handle 为 NULL 时，函数不做任何操作
 */
void task_delete(TaskHandle_t *handle);

#endif /* TASK_SET_H */