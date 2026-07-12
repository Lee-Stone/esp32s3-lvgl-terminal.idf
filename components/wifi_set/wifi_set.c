#include <stdio.h>
#include "wifi_set.h"
#include "task_set.h"

static void wifi_callback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // WiFi 驱动启动成功
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        app.setting.wifi_status = WL_START;
    }
    // WiFi 连接断开 / 连接失败 → 更新 UI 并计数
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        app.setting.wifi_status = WL_DISCONNECTED;
        app.setting.wifi_failure++;

        if (esp_lv_adapter_lock(-1) == ESP_OK) {
            lv_label_set_text(ui_LabelWiFiState, "未连接");
            lv_obj_clear_state(ui_SwitchWiFi, LV_STATE_CHECKED);
            if (app.setting.wifi_failure >= 2) {
                lv_obj_add_flag(ui_SpinnerWiFi, LV_OBJ_FLAG_HIDDEN);
            }

            // 失能小智启动按钮
			lv_obj_add_state(ui_SwitchXiaoZhiSpeak, LV_STATE_DISABLED);
			lv_obj_clear_state(ui_SwitchXiaoZhiSpeak, LV_STATE_CHECKED);
            lv_label_set_text(ui_LabelXiaoZhiSpeak, "OFF");
			lv_textarea_set_text(ui_TextAreaXiaozhiQuestion, "请连接WiFi后使用小智功能哦~");
			lv_textarea_set_text(ui_TextAreaXiaoZhiAnswer, "请连接WiFi后使用小智功能哦~");

            esp_lv_adapter_unlock();
        }
    }
    // DHCP 获取到 IP → 连接成功，触发首次时间/天气初始化
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        app.setting.wifi_status = WL_CONNECTED;
        app.setting.wifi_failure = 0;

        if (esp_lv_adapter_lock(-1) == ESP_OK) {
            wifi_config_t conf;
            esp_wifi_get_config(WIFI_IF_STA, &conf);
            lv_label_set_text_fmt(ui_LabelWiFiState, "已连接 %s", conf.sta.ssid);
            lv_obj_add_state(ui_SwitchWiFi, LV_STATE_CHECKED);
            lv_obj_add_flag(ui_SpinnerWiFi, LV_OBJ_FLAG_HIDDEN);

            // WiFi 连接成功，初始化时间/天气/小智
            if (!app.data.time_init) {
                app.data.time_init = true;
                data_init();
            }

            if (!app.data.weather_update) {
                app.data.weather_update = true;
            }

            if (!app.xiaozhi.xiaozhi_start) {
                app.xiaozhi.xiaozhi_start = true;
            }

            esp_lv_adapter_unlock();
        }
    }
}

void wifi_init(void)
{
    // (1) 初始化 NVS（WiFi 库内部依赖，存射频校准 & 连接记录）
    nvs_flash_init();

    // (2) 初始化 TCP/IP 协议栈（LwIP 核心任务）
    esp_netif_init();

    // (3) 创建默认事件循环（WiFi 异步通知机制）
    esp_event_loop_create_default();

    // (4) 创建 STA 网络接口（客户端模式）
    esp_netif_create_default_wifi_sta();

    // (5) 注册 WiFi 事件回调
    esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_START, wifi_callback, NULL, NULL);
    esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_callback, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_callback, NULL, NULL);

    // (6) 初始化 WiFi 硬件驱动（分配 RX/TX 缓冲区，启动 WiFi 任务）
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
}

void wifi_begin(const char *ssid, const char *password)
{
    // (7) 设置 WiFi 为 STA 模式
    esp_wifi_set_mode(WIFI_MODE_STA);

    // (8) 填入用户传入的 SSID 和密码
    wifi_config_t wifi_config = (wifi_config_t){
        .sta = {
            .ssid = "",
            .password = "",
            .bssid_set = false,
        }
    };
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

    // (9) 启动 WiFi
    esp_wifi_start();

    // (10) 开始连接（异步，结果通过回调通知）
    esp_wifi_connect();
}


void wifi_task(void *pvParameters)
{
    while (1) {
        if (app.setting.wifi_scan == true) {
 			// 设置 WiFi 为 STA 模式
			esp_wifi_set_mode(WIFI_MODE_STA);

			// 启动 WiFi
			esp_wifi_start();

			// 断开连接才能启动扫描
			esp_wifi_disconnect(); 

            // 启动扫描（阻塞当前任务，扫描完成后返回）
            esp_wifi_scan_start(NULL, true);

            // 获取扫描到的 AP 数量
            uint16_t ap_count = 0;
            esp_wifi_scan_get_ap_num(&ap_count);

            if (ap_count == 0) {
                // 没有 WiFi → 显示提示文字
                free(app.setting.wifi_networks);
                app.setting.wifi_networks = malloc(32);
                strcpy(app.setting.wifi_networks, "附近无可用WiFi\n");
            } else {
                // 获取 AP 记录
                wifi_ap_record_t *ap_info = malloc(sizeof(wifi_ap_record_t) * ap_count);
                esp_wifi_scan_get_ap_records(&ap_count, ap_info);

                // 计算拼接后的总长度（每个 SSID + \n + 结尾 \0）
                size_t total_len = 0;  
                for (int i = 0; i < ap_count; i++) {
                    total_len += strlen((char *)ap_info[i].ssid) + 2; 
                }

                // 拼接所有 SSID 为一行一个
                free(app.setting.wifi_networks);
                app.setting.wifi_networks = malloc(total_len);
                app.setting.wifi_networks[0] = '\0';
                for (int i = 0; i < ap_count; i++) {
                    strcat(app.setting.wifi_networks, (char *)ap_info[i].ssid);
                    strcat(app.setting.wifi_networks, "\n");
                }
				// 去掉最后一个换行符
				app.setting.wifi_networks[total_len - 1] = '\0';  

                // 默认选中第一个
                strncpy(app.setting.wifi_ssid, (char *)ap_info[0].ssid, sizeof(app.setting.wifi_ssid) - 1);

                free(ap_info);
            }

			// 显示扫描结果
			if (esp_lv_adapter_lock(-1) == ESP_OK) {
				lv_roller_set_options(ui_RollerWiFi, app.setting.wifi_networks, LV_ROLLER_MODE_NORMAL);
				esp_lv_adapter_unlock();
			}

			// 扫描完成尝试连接
			esp_wifi_connect();

            app.setting.wifi_scan = false;
        }
        else if(app.setting.wifi_connect == true) {
            // UI 触发：用选中的 SSID 和密码连接
            esp_wifi_disconnect();
            wifi_begin(app.setting.wifi_ssid, app.setting.wifi_password);
            app.setting.wifi_connect = false;
        }
        else if(app.setting.wifi_disconnect == true) {
            // UI 触发：断开连接
            esp_wifi_disconnect();
            app.setting.wifi_disconnect = false;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}