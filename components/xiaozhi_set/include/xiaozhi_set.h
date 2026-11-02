#ifndef XIAOZHI_SET_H
#define XIAOZHI_SET_H

#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "driver/i2s_std.h"
#include "esp_xiaozhi_chat.h"
#include "esp_xiaozhi_info.h"
#include "esp_mcp_engine.h"
#include "esp_event.h"
#include "audio_mixer.h"
#include "audio_stream.h"
#include "opus.h"

// I2S 数字麦克风(MSM261) 引脚（按原理图 Schematic1_1 网络表逐项核对）
#define MIC_I2S_SCK       GPIO_NUM_11   // I_SCK
#define MIC_I2S_WS        GPIO_NUM_13   // I_WS
#define MIC_I2S_DIN       GPIO_NUM_12   // I_SD

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