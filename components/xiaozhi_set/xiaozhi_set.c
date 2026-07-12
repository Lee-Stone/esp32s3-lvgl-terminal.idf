#include <stdio.h>
#include "xiaozhi_set.h"
#include "task_set.h"

static i2s_chan_handle_t pdm_rx_chan = NULL;
static esp_xiaozhi_chat_handle_t chat_hd = 0;
static audio_stream_handle_t tts_stream = NULL;
static OpusDecoder *opus_dec = NULL;
static int16_t *opus_buf = NULL;
static int16_t *out_buf = NULL;
static esp_xiaozhi_chat_audio_t pcm_params = {
    .format = "pcm",
    .sample_rate = 48000,
    .channels = 2,
    .frame_duration = 60,
};

// 对话事件回调：显示对话文本，TTS 结束后自动重新监听
static void xiaozhi_chat_event(esp_xiaozhi_chat_event_t event, void *event_data, void *ctx)
{
    switch (event) {
        case ESP_XIAOZHI_CHAT_EVENT_CHAT_TEXT: {
            esp_xiaozhi_chat_text_data_t *chat = event_data;
            if (chat->role == ESP_XIAOZHI_CHAT_TEXT_ROLE_USER) {
                if (esp_lv_adapter_lock(-1) == ESP_OK) {
                    lv_textarea_set_text(ui_TextAreaXiaozhiQuestion, chat->text);
                    esp_lv_adapter_unlock();
                }
            }
            else if (chat->role == ESP_XIAOZHI_CHAT_TEXT_ROLE_ASSISTANT) {
                if (esp_lv_adapter_lock(-1) == ESP_OK) {
                    lv_textarea_set_text(ui_TextAreaXiaoZhiAnswer, chat->text);
                    esp_lv_adapter_unlock();
                }
            }
            break;
        }
        case ESP_XIAOZHI_CHAT_EVENT_CHAT_TTS_STATE: {
            esp_xiaozhi_chat_tts_state_t *tts = event_data;
            if (tts->state == ESP_XIAOZHI_CHAT_TTS_STATE_STOP) {
                esp_xiaozhi_chat_send_start_listening(chat_hd, ESP_XIAOZHI_CHAT_LISTENING_MODE_AUTO);
            }
            break;
        }
        default:
            break;
    }
}

// 连接事件回调：打开音频通道、发送唤醒词、处理断开
static void xiaozhi_connect_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base != ESP_XIAOZHI_CHAT_EVENTS) return;

    switch (id) {
        case ESP_XIAOZHI_CHAT_EVENT_CONNECTED:
            esp_xiaozhi_chat_open_audio_channel(chat_hd, &pcm_params, NULL, 0);
            break;
        case ESP_XIAOZHI_CHAT_EVENT_DISCONNECTED:
            if (app.xiaozhi.xiaozhi_status == 3) {
                esp_xiaozhi_chat_stop(chat_hd);
            }
            if (esp_lv_adapter_lock(-1) == ESP_OK) {
                lv_obj_clear_state(ui_SwitchXiaoZhiSpeak, LV_STATE_CHECKED);
                lv_label_set_text(ui_LabelXiaoZhiSpeak, "OFF");
                esp_lv_adapter_unlock();
            }
            app.xiaozhi.xiaozhi_status = 0;
            break;
        case ESP_XIAOZHI_CHAT_EVENT_AUDIO_CHANNEL_OPENED:
            esp_xiaozhi_chat_send_wake_word(chat_hd, "你好小智");
            esp_xiaozhi_chat_send_start_listening(chat_hd, ESP_XIAOZHI_CHAT_LISTENING_MODE_AUTO);
            app.xiaozhi.xiaozhi_status = 3;
            break;
    }
}

// 音频回调：OPUS 解码 → 48kHz PCM → 44.1kHz 立体声 → 混音器
static void xiaozhi_audio_event(const uint8_t *data, int len, void *ctx)
{
    if (!tts_stream || !data || len <= 0) return;
    if (!opus_dec) {
        int err;
        opus_dec = opus_decoder_create(48000, 1, &err);
        if (!opus_dec) return;
        opus_buf = heap_caps_malloc(5760 * 2, MALLOC_CAP_SPIRAM);
        out_buf = heap_caps_malloc(5760 * 4, MALLOC_CAP_SPIRAM);
    }
    if (!opus_buf || !out_buf) return;

    int n = opus_decode(opus_dec, data, len, opus_buf, 5760, 0);
    if (n <= 0) return;

    int out_n = 0;
    for (int i = 0; i < n; i++) {
        int j = (int64_t)i * 44100 / 48000;
        if (j >= out_n) {
            out_buf[out_n * 2] = out_buf[out_n * 2 + 1] = opus_buf[i];
            out_n++;
        }
    }
    audio_stream_write_pcm(tts_stream, out_buf, out_n * 4, pdMS_TO_TICKS(100));
}

void xiaozhi_init(void)
{
    // (1) I2S0 PDM RX 麦克风
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, NULL, &pdm_rx_chan);

    i2s_pdm_rx_config_t pdm_cfg = {
        .clk_cfg  = I2S_PDM_RX_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = XIAOZHI_PDM_CLK,
            .din = XIAOZHI_PDM_DATA,
        },
    };
    i2s_channel_init_pdm_rx_mode(pdm_rx_chan, &pdm_cfg);
    i2s_channel_enable(pdm_rx_chan);

    // (2) 初始化小智对话
    esp_mcp_t *mcp = NULL;
    esp_mcp_create(&mcp);

    esp_xiaozhi_chat_config_t chat_cfg = {
        .audio_type      = ESP_XIAOZHI_CHAT_AUDIO_TYPE_OPUS,
        .audio_callback  = xiaozhi_audio_event,
        .event_callback  = xiaozhi_chat_event,
        .mcp_engine      = mcp,
        .owns_mcp_engine = true,
    };
    esp_xiaozhi_chat_init(&chat_cfg, &chat_hd);

    // (3) 创建 TTS 输出流（RAW 类型，PCM 写入混音器共喇叭）
    audio_stream_config_t stream_cfg = {
        .type     = AUDIO_STREAM_TYPE_RAW,
        .name     = "tts",
        .priority = 9,
        .coreID   = 1,
    };
    tts_stream = audio_stream_new(&stream_cfg);

    // (4) 注册连接/断开事件
    esp_event_handler_register(ESP_XIAOZHI_CHAT_EVENTS, ESP_EVENT_ANY_ID, xiaozhi_connect_event, NULL);
}

void xiaozhi_task(void *pvParameters)
{
    uint8_t pdm_buf[2048] = {0};

    while (1) {
        if (app.xiaozhi.xiaozhi_start) {
            // 获取小智激活码 若没有激活则等待小智激活
            esp_xiaozhi_chat_info_t info = {0};
            esp_xiaozhi_chat_get_info(&info);
            while (info.has_activation_code) {
                if (esp_lv_adapter_lock(-1) == ESP_OK) {
                    // 失能小智启动按钮
                    lv_label_set_text(ui_LabelXiaoZhiSpeak, "OFF");
                    lv_obj_clear_state(ui_SwitchXiaoZhiSpeak, LV_STATE_CHECKED);
                    lv_obj_add_state(ui_SwitchXiaoZhiSpeak, LV_STATE_DISABLED);
                    lv_textarea_set_text(ui_TextAreaXiaozhiQuestion, "请访问 xiaozhi.me 输入以下6位验证码激活设备");
                    lv_textarea_set_text(ui_TextAreaXiaoZhiAnswer, info.activation_code);
                    esp_lv_adapter_unlock();
                }
                // 每秒重新获取一次激活状态
                vTaskDelay(pdMS_TO_TICKS(1000));
                info = (esp_xiaozhi_chat_info_t){0};
                esp_xiaozhi_chat_free_info(&info);
                esp_xiaozhi_chat_get_info(&info);
            }
            esp_xiaozhi_chat_free_info(&info);

            if (esp_lv_adapter_lock(-1) == ESP_OK) {
                // 使能小智启动按钮
                lv_obj_clear_state(ui_SwitchXiaoZhiSpeak, LV_STATE_DISABLED);
                lv_textarea_set_text(ui_TextAreaXiaozhiQuestion, "点击按钮开始和小智对话~");
                lv_textarea_set_text(ui_TextAreaXiaoZhiAnswer, "点击按钮开始和小智对话~");
                esp_lv_adapter_unlock();
            }

            app.xiaozhi.xiaozhi_start = false;
        }

        // ui_SwitchXiaoZhiSpeak 状态到了 ON
        if (app.xiaozhi.xiaozhi_status == 1) {
            esp_xiaozhi_chat_start(chat_hd);
            app.xiaozhi.xiaozhi_status = 0;
        }
        // ui_SwitchXiaoZhiSpeak 状态到了 OFF
        else if (app.xiaozhi.xiaozhi_status == 2) {
            esp_xiaozhi_chat_stop(chat_hd);
            app.xiaozhi.xiaozhi_status = 0;
        }
        // 启动成功，开始对话
        else if (app.xiaozhi.xiaozhi_status == 3) {
            size_t bytes_read = 0;
            i2s_channel_read(pdm_rx_chan, pdm_buf, sizeof(pdm_buf), &bytes_read, pdMS_TO_TICKS(60));
            if (bytes_read > 0) {
                esp_xiaozhi_chat_send_audio_data(chat_hd, (const char *)pdm_buf, bytes_read);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(60));
    }
}