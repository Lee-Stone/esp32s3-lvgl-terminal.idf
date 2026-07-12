#include <stdio.h>
#include "lcd_set.h"

void lcd_init(lcd_dev_t *lcd_dev)
{
    // 初始化 SPI 总线
    spi_bus_config_t bus_cfg = {
        .sclk_io_num = LCD_SCK_PIN,
        .mosi_io_num = LCD_MOSI_PIN,
        .miso_io_num = LCD_MISO_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * sizeof(uint16_t),
    };
    spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);

    // 创建 SPI LCD 面板 IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_DC_PIN,
        .cs_gpio_num = LCD_CS_PIN,
        .pclk_hz = 40 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 4,
    };
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle);

    // 创建 LCD 面板（ST7789）
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_PIN,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
    };
    esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);

    // 初始化面板
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);

    // 设置横屏
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, false, true);

    // 开启显示
    esp_lcd_panel_disp_on_off(panel_handle, true);

    // 配置背光 PWM（LEDC）
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LCD_BL_LEDC_MODE,
        .duty_resolution = LCD_BL_LEDC_DUTY_RES,
        .timer_num       = LCD_BL_LEDC_TIMER,
        .freq_hz         = LCD_BL_LEDC_FREQ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num   = LCD_LED_PIN,
        .speed_mode = LCD_BL_LEDC_MODE,
        .channel    = LCD_BL_LEDC_CHANNEL,
        .timer_sel  = LCD_BL_LEDC_TIMER,
        .duty       = LCD_BL_DUTY_MAX,   // 初始最亮
        .hpoint     = 0,
    };
    ledc_channel_config(&ledc_channel);

    lcd_dev->panel = panel_handle;
    lcd_dev->panel_io = io_handle;
}

void lcd_set_brightness(uint8_t percent)
{
    if (percent > 100) percent = 100;
    uint32_t duty = (uint32_t)percent * LCD_BL_DUTY_MAX / 100;
    ledc_set_duty(LCD_BL_LEDC_MODE, LCD_BL_LEDC_CHANNEL, duty);
    ledc_update_duty(LCD_BL_LEDC_MODE, LCD_BL_LEDC_CHANNEL);
}