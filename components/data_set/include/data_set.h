#ifndef DATA_SET_H
#define DATA_SET_H

#include "esp_sntp.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"

// 心知天气 API 参数
#define WEATHER_KEY         "Sq9D2GX9Rie6Gljzv"
#define WEATHER_LOCATION    "shenzhen"
#define WEATHER_LANGUAGE    "zh-Hans"
#define WEATHER_UNIT        "c"
#define WEATHER_URL_NOW     "https://api.seniverse.com/v3/weather/now.json"
#define WEATHER_URL_DAILY   "https://api.seniverse.com/v3/weather/daily.json"
#define WEATHER_START       "0"
#define WEATHER_DAYS        "3"

/**
 * @brief 初始化 SNTP 时间同步
 */
void data_init(void);

/**
 * @brief 数据后台任务（时间刷新 / 天气获取）
 */
void data_task(void *pvParameters);

#endif /* DATA_SET_H */