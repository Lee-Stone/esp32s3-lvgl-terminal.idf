#ifndef XIAOZHI_SET_H
#define XIAOZHI_SET_H

#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "driver/i2s_pdm.h"
#include "esp_xiaozhi_chat.h"
#include "esp_xiaozhi_info.h"
#include "esp_mcp_engine.h"
#include "esp_event.h"
#include "audio_mixer.h"
#include "audio_stream.h"
#include "opus.h"

// PDM 麦克风引脚
#define XIAOZHI_PDM_CLK   GPIO_NUM_38
#define XIAOZHI_PDM_DATA  GPIO_NUM_39

/**
 * @brief 小智对话初始化
 *
 * 初始化 PDM 麦克风、OPUS 解码器、MCP 引擎、TTS 输出流，注册连接与对话事件回调。
 * 应在 WiFi 连接成功后调用。
 */
void xiaozhi_init(void);

/**
 * @brief 小智后台任务
 *
 * 处理设备激活、Speak 开关状态机、PDM 麦克风音频采集与发送。
 *
 * @param pvParameters  FreeRTOS 任务参数（未使用）
 */
void xiaozhi_task(void *pvParameters);

#endif /* XIAOZHI_SET_H */