#ifndef MUSIC_SET_H
#define MUSIC_SET_H

#include "driver/sdspi_host.h"
#include "driver/i2s_std.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "esp_random.h"
#include "audio_mixer.h"
#include "audio_stream.h"

// SD 卡 SPI 引脚（按原理图 Schematic1_1 网络表逐项核对）
#define SD_CS              GPIO_NUM_9
#define SD_MOSI            GPIO_NUM_10
#define SD_MISO            GPIO_NUM_17
#define SD_SCK             GPIO_NUM_14

// I2S 数字功放引脚
#define I2S_BCLK           GPIO_NUM_40   // O_BCLK
#define I2S_LRC            GPIO_NUM_41   // O_LRC
#define I2S_DIN            GPIO_NUM_39   // O_DIN

/**
 * @brief 音乐播放器初始化
 */
void music_init(void);

/**
 * @brief 扫描 SD 卡中的 .mp3 文件
 */
void music_read(void);

/**
 * @brief 音乐后台任务
 */
void music_task(void *pvParameters);

#endif /* MUSIC_SET_H */
