#ifndef WIFI_SET_H
#define WIFI_SET_H

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#define WL_START            2
#define WL_CONNECTED        1
#define WL_DISCONNECTED     0

/**
 * @brief WiFi 一次性初始化（NVS / netif / 事件循环 / STA 接口 / 事件注册 / wifi_init）
 *
 * 应在 app_main() 中调用一次，对标 Arduino 的 Network.begin() + wifiLowLevelInit()
 */
void wifi_init(void);

/**
 * @brief WiFi 连接
 *
 * 启动异步连接，结果通过回调通知
 *
 * @param ssid      WiFi 名称
 * @param password  WiFi 密码
 */
void wifi_begin(const char *ssid, const char *password);

/**
 * @brief WiFi 后台任务（扫描 / 状态更新）
 */
void wifi_task(void *pvParameters);

#endif