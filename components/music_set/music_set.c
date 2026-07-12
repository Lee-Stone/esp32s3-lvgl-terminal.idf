#include <stdio.h>
#include "music_set.h"
#include "task_set.h"
#include <string.h>
#include <dirent.h>

static i2s_chan_handle_t i2s_tx_chan = NULL;
static audio_stream_handle_t audio_stream = NULL;

// I2S 输出回调
static esp_err_t audio_i2s_write(void *buf, size_t len, size_t *written, uint32_t timeout)
{
    if (app.music.music_volume != 100) {
        int16_t *volum_buf = (int16_t *)buf;
        for (size_t i = 0; i < (len / 2); i++) {
            volum_buf[i] = (int16_t)(((int32_t)volum_buf[i] * app.music.music_volume) / 100);
        }
    }
    return i2s_channel_write(i2s_tx_chan, buf, len, written, timeout);
}

// I2S 时钟切换回调
static esp_err_t audio_i2s_clk_set(uint32_t rate, uint32_t bits, i2s_slot_mode_t ch)
{
    i2s_channel_disable(i2s_tx_chan);
    i2s_std_clk_config_t clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(rate);
    i2s_channel_reconfig_std_clock(i2s_tx_chan, &clk_cfg);
    i2s_channel_enable(i2s_tx_chan);
    return ESP_OK;
}

void music_read(void)
{
    DIR *root = opendir("/sdcard");
    if (root == NULL) {
        if (esp_lv_adapter_lock(-1) == ESP_OK) {
            lv_roller_set_options(ui_RollerMusic, "SD卡未挂载\n", LV_ROLLER_MODE_NORMAL);
            esp_lv_adapter_unlock();
        }
        return;
    }

    struct dirent *file;
    int cnt = 0;
    size_t list_len = 0;

    while ((file = readdir(root)) != NULL) {
        const char *filename = file->d_name;
        if (filename[0] == '.') continue;
        if (file->d_type == DT_DIR) continue;

        if (cnt <= 1) cnt++;
        int len = strlen(filename);
        const char *mp3_ext = ".mp3";
        if (strcasecmp(mp3_ext, &filename[len - 4]) == 0) {
            if (cnt == 1) {
                memset(app.music.music_name, 0, sizeof(app.music.music_name));
                strcpy(app.music.music_name, filename);
            }
            app.music.music_list = realloc(app.music.music_list, list_len + len + 2);
            strcpy(app.music.music_list + list_len, filename);
            list_len += len;
            app.music.music_list[list_len++] = '\n';
            app.music.music_list[list_len] = '\0';
        }
    }
    app.music.music_list[list_len - 1] = '\0';
    closedir(root);

    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        if (app.music.music_list != NULL) {
            lv_roller_set_options(ui_RollerMusic, app.music.music_list, LV_ROLLER_MODE_NORMAL);
        } else {
            lv_roller_set_options(ui_RollerMusic, "SD卡根目录下无可用音乐文件\n", LV_ROLLER_MODE_NORMAL);
        }
        esp_lv_adapter_unlock();
    }
}

void music_init(void)
{
    // (1) 配置 SPI3 总线（SD 卡）
    spi_bus_config_t bus_cfg = {
        .mosi_io_num     = SD_MOSI,
        .miso_io_num     = SD_MISO,
        .sclk_io_num     = SD_SCK,
        .quadwp_io_num   = GPIO_NUM_NC,
        .quadhd_io_num   = GPIO_NUM_NC,
        .max_transfer_sz = 4092,
    };
    spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO);

    // (2) 配置 SDSPI 设备
    sdspi_device_config_t slot_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_cfg.gpio_cs = SD_CS;
    slot_cfg.host_id = SPI3_HOST;

    // (3) 挂载 FAT 文件系统
    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files              = 5,
        .allocation_unit_size   = 16 * 1024,
    };
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI3_HOST;
    esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_cfg, &mount_cfg, NULL);

    // (4) 扫描歌曲
    music_read();

    // (5) 初始化 I2S（标准 Philips 模式，44.1kHz 16bit 立体声）
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, &i2s_tx_chan, NULL);
    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCLK,
            .ws   = I2S_LRC,
            .dout = I2S_DIN,
            .din  = I2S_GPIO_UNUSED,
        },
    };
    i2s_channel_init_std_mode(i2s_tx_chan, &std_cfg);
    i2s_channel_enable(i2s_tx_chan);

    // (6) 初始化音频混音器
    audio_mixer_config_t mixer_cfg = {
        .write_fn   = audio_i2s_write,
        .clk_set_fn = audio_i2s_clk_set,
        .priority   = 10,
        .coreID     = 1,
        .i2s_format = { .sample_rate = 44100, .bits_per_sample = 16, .channels = 2 },
    };
    audio_mixer_init(&mixer_cfg);

    // (7) 创建解码流
    audio_stream_config_t stream_cfg = {
        .type     = AUDIO_STREAM_TYPE_DECODER,
        .name     = "music",
        .priority = 9,
        .coreID   = 1,
    };
    audio_stream = audio_stream_new(&stream_cfg);
}

void music_play(const char *filename)
{
    if (filename == NULL || filename[0] == '\0') return;

    char path[256];
    snprintf(path, sizeof(path), "/sdcard/%s", filename);
    FILE *fp = fopen(path, "rb");
    if (fp) {
        audio_stream_play(audio_stream, fp);
    }
}

void music_task(void *pvParameters)
{
    while (1) {
        if (app.music.music_play) {
            // 暂停->播放
            if (app.music.music_pause && audio_stream_get_state(audio_stream) == AUDIO_PLAYER_STATE_PAUSE) {
                if (esp_lv_adapter_lock(-1) == ESP_OK) {
                    lv_label_set_text_fmt(ui_LabelMusicName, "正在播放: %s", app.music.music_name);
                    esp_lv_adapter_unlock();
                }
                audio_stream_resume(audio_stream);
                app.music.music_pause = false;
            }
            // 切歌
            if (app.music.music_start) {
                if (esp_lv_adapter_lock(-1) == ESP_OK) {
                    lv_roller_get_selected_str(ui_RollerMusic, app.music.music_name, sizeof(app.music.music_name));
                    lv_label_set_text_fmt(ui_LabelMusicName, "正在播放: %s", app.music.music_name);
                    app.music.music_index = lv_roller_get_selected(ui_RollerMusic);
                    lv_obj_add_state(ui_StartMusic, LV_STATE_CHECKED);
                    esp_lv_adapter_unlock();
                }
                music_play(app.music.music_name);
                while (audio_stream_get_state(audio_stream) == AUDIO_PLAYER_STATE_IDLE) {
                    vTaskDelay(pdMS_TO_TICKS(200));
                }
                app.music.music_start = false;
            }
            // 播放完成后自动切歌
            if (!app.music.music_start && audio_stream_get_state(audio_stream) == AUDIO_PLAYER_STATE_IDLE) {
                if (app.music.music_mode == 0) {
                    // 单曲循环，不操作
                }
                else if (app.music.music_mode == 1) {
                    // 顺序播放
                    if (app.music.music_index == lv_roller_get_option_cnt(ui_RollerMusic) - 1) {
                        app.music.music_index = 0;
                    } else {
                        app.music.music_index++;
                    }
                } else if (app.music.music_mode == 2) {
                    // 随机播放
                    app.music.music_index = esp_random() % (lv_roller_get_option_cnt(ui_RollerMusic) - 1);
                }

                if (esp_lv_adapter_lock(-1) == ESP_OK) {
                    lv_roller_set_selected(ui_RollerMusic, app.music.music_index, LV_ANIM_OFF);
                    esp_lv_adapter_unlock();
                }

                app.music.music_start = true;
            }
        }
        else {
            // 播放->暂停
            if (!app.music.music_pause && audio_stream_get_state(audio_stream) == AUDIO_PLAYER_STATE_PLAYING) {
                if (esp_lv_adapter_lock(-1) == ESP_OK) {
                    lv_label_set_text_fmt(ui_LabelMusicName, "暂停播放: %s", app.music.music_name);
                    esp_lv_adapter_unlock();
                }
                audio_stream_pause(audio_stream);
                app.music.music_pause = true;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}