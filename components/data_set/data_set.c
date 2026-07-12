#include <stdio.h>
#include "data_set.h"
#include "task_set.h"

void data_init(void)
{
    // (1) 设置 SNTP 轮询模式
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    // (2) 设置阿里云 NTP 服务器
    sntp_setservername(0, "ntp1.aliyun.com");
    sntp_setservername(1, "ntp2.aliyun.com");
    sntp_setservername(2, "ntp3.aliyun.com");

    // (3) 启动 SNTP
    sntp_init();

    // (4) 设置时区（北京时间 UTC+8）
    setenv("TZ", "CST-8", 1);
    tzset();
}

void data_task(void *pvParameters)
{
    while (1) {
        if (app.data.time_init) {
            time_t now;
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            static const char *week[] = {"星期日","星期一","星期二","星期三","星期四","星期五","星期六"};

            // 时间已同步 → 更新 UI
            if ((timeinfo.tm_year + 1900) >= 2026) {
                if (esp_lv_adapter_lock(-1) == ESP_OK) {
                    lv_label_set_text_fmt(ui_LabelTime1, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                    lv_label_set_text_fmt(ui_LabelTime2, "%d/%02d/%02d %s", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, week[timeinfo.tm_wday]);
                    lv_calendar_set_today_date(ui_Calendar, timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
                    lv_calendar_set_showed_date(ui_Calendar, timeinfo.tm_year + 1900, timeinfo.tm_mon + 1);
                    esp_lv_adapter_unlock();
                }
            }

            if (app.data.weather_update) {
                char url[512] = {0};
                char buf[2048] = {0};
                esp_http_client_config_t cfg = {0};
                esp_http_client_handle_t client = {0};
                cJSON *now = NULL;
                cJSON *daily = NULL;
                cJSON *result = NULL;

                // (1) 获取当前天气（心知天气 now.json API）
                snprintf(url, sizeof(url), "%s?key=%s&location=%s&language=%s&unit=%s", WEATHER_URL_NOW, WEATHER_KEY, WEATHER_LOCATION, WEATHER_LANGUAGE, WEATHER_UNIT);
                cfg.url = url;
                cfg.skip_cert_common_name_check = true;       // 跳过域名验证
                cfg.crt_bundle_attach = esp_crt_bundle_attach; // 启用内置 CA 证书

                client = esp_http_client_init(&cfg);
                esp_http_client_set_method(client, HTTP_METHOD_GET);
                
                if(esp_http_client_open(client, 0) == ESP_OK) {
                    int data_len = esp_http_client_fetch_headers(client);
                    if (data_len >= 0) {
                        int data_read = esp_http_client_read(client, buf, sizeof(buf) - 1);
                        if (data_read >= 0) {
                            now = cJSON_Parse(buf);                    // 解析 JSON
                            result = cJSON_GetArrayItem(cJSON_GetObjectItem(now, "results"), 0);
                            if (result) {
                                if (esp_lv_adapter_lock(-1) == ESP_OK) {
                                    // 当前天气
                                    lv_label_set_text_fmt(ui_LabelLocation, "地点:%s", cJSON_GetObjectItem(cJSON_GetObjectItem(result, "location"), "name")->valuestring);
                                    lv_label_set_text_fmt(ui_LabelTemp, "温度:%s度", cJSON_GetObjectItem(cJSON_GetObjectItem(result, "now"), "temperature")->valuestring);
                                    lv_label_set_text_fmt(ui_LabelCode, "天气:%s", cJSON_GetObjectItem(cJSON_GetObjectItem(result, "now"), "text")->valuestring);
                                    lv_label_set_text_fmt(ui_LabelUpdateTime, "上次更新时间:%s", cJSON_GetObjectItem(result, "last_update")->valuestring);
                                    esp_lv_adapter_unlock();
                                }
                            }
                        }
                    }
                }
                esp_http_client_close(client);
                esp_http_client_cleanup(client);

                // (2) 获取未来天气（心知天气 daily.json API）
                snprintf(url, sizeof(url), "%s?key=%s&location=%s&language=%s&unit=%s&start=%s&days=%s", WEATHER_URL_DAILY, WEATHER_KEY, WEATHER_LOCATION, WEATHER_LANGUAGE, WEATHER_UNIT, WEATHER_START, WEATHER_DAYS);
                cfg.url = url;
                cfg.skip_cert_common_name_check = true;
                cfg.crt_bundle_attach = esp_crt_bundle_attach;
                client = esp_http_client_init(&cfg);
                esp_http_client_set_method(client, HTTP_METHOD_GET);
                
                if(esp_http_client_open(client, 0) == ESP_OK) {
                    int data_len = esp_http_client_fetch_headers(client);
                    if (data_len >= 0) {
                        int data_read = esp_http_client_read(client, buf, sizeof(buf) - 1);
                        if (data_read >= 0) {
                            daily = cJSON_Parse(buf);                    // 解析每日天气 JSON
                            result = cJSON_GetArrayItem(cJSON_GetObjectItem(daily, "results"), 0);
                            if (result) {
                                if (esp_lv_adapter_lock(-1) == ESP_OK) {
                                    // 未来三天滚动列表 + 明天天气（在循环中一并处理）
                                    result = cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(daily, "results"), 0), "daily");
                                    buf[0] = '\0';
                                    for (int i = 0; i < 3; i++) {
                                        const char *date = (i == 0) ? "今天" : week[(timeinfo.tm_wday + i <= 6) ? (timeinfo.tm_wday + i) : (timeinfo.tm_wday + i - 7)];
                                        // i==1 时顺便更新明天天气标签
                                        if (i == 1) {
                                            lv_label_set_text_fmt(ui_LabelTomorrowCode,
                                                "明天%s,天气%s,温度%s~%s度,降水概率%s",
                                                week[(timeinfo.tm_wday + 1 <= 6) ? (timeinfo.tm_wday + 1) : 0],
                                                cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "text_day")->valuestring,
                                                cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "low")->valuestring,
                                                cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "high")->valuestring,
                                                cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "precip")->valuestring);
                                        }
                                        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1,
                                            "%s %s 温度:%s~%s 降水:%s\n", date,
                                            cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "text_day")->valuestring,
                                            cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "low")->valuestring,
                                            cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "high")->valuestring,
                                            cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "precip")->valuestring);
                                    }
                                    lv_roller_set_options(ui_RollerFuture, buf, LV_ROLLER_MODE_NORMAL);
                                    esp_lv_adapter_unlock();
                                }
                            }
                        }
                    }
                }
                esp_http_client_close(client);
                esp_http_client_cleanup(client);

                cJSON_Delete(now);
                cJSON_Delete(daily);

                app.data.weather_update = false;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}




