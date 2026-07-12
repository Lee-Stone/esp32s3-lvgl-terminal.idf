#ifndef LCD_SET_H
#define LCD_SET_H

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_lcd_types.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

// LCD 引脚定义
#define LCD_SCK_PIN     GPIO_NUM_12
#define LCD_MOSI_PIN    GPIO_NUM_11
#define LCD_MISO_PIN    GPIO_NUM_13
#define LCD_CS_PIN      GPIO_NUM_10
#define LCD_DC_PIN      GPIO_NUM_9
#define LCD_LED_PIN     GPIO_NUM_14
#define LCD_RST_PIN     GPIO_NUM_NC

// LCD 分辨率
#define LCD_WIDTH       320
#define LCD_HEIGHT      240

// 背光 PWM 参数
#define LCD_BL_LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LCD_BL_LEDC_TIMER      LEDC_TIMER_0
#define LCD_BL_LEDC_CHANNEL    LEDC_CHANNEL_0
#define LCD_BL_LEDC_DUTY_RES   LEDC_TIMER_13_BIT
#define LCD_BL_LEDC_FREQ       5000
#define LCD_BL_DUTY_MAX        ((1 << 13) - 1)   // 13位分辨率: 0~8191

// LCD 句柄类型
typedef struct {
    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_io_handle_t panel_io;
} lcd_dev_t;

void lcd_init(lcd_dev_t *lcd_dev);

/**
 * @brief 设置 LCD 背光亮度
 *
 * @param percent 亮度百分比 (0~100)，0=灭，100=最亮
 */
void lcd_set_brightness(uint8_t percent);

void lcd_flush(lcd_dev_t *lcd_dev, int x_start, int y_start,
               int x_end, int y_end, const uint16_t *color_data);
void lcd_fill_color(lcd_dev_t *lcd_dev, uint16_t color);

#endif /* LCD_SET_H */