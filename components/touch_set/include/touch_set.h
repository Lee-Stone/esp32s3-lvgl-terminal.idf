#ifndef TOUCH_SET_H
#define TOUCH_SET_H

#include "driver/gpio.h"
#include "softspi_master.h"
#include "esp_lcd_touch_xpt2046.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_io_interface.h"

// XPT2046 引脚定义
#define TOUCH_CLK_PIN   GPIO_NUM_15
#define TOUCH_CS_PIN    GPIO_NUM_7
#define TOUCH_DIN_PIN   GPIO_NUM_6   // MOSI
#define TOUCH_DO_PIN    GPIO_NUM_5   // MISO
#define TOUCH_IRQ_PIN   GPIO_NUM_4   // 中断引脚

// 屏幕分辨率
#define TOUCH_WIDTH     320
#define TOUCH_HEIGHT    240

// 触摸设备结构体
typedef struct {
    uint16_t x;
    uint16_t y;
    bool pressed;
    esp_lcd_touch_handle_t handle;
} touch_dev_t;

/**
 * @brief 初始化触摸设备
 *
 * @param touch_dev_t *dev
 */
void touch_init(touch_dev_t *dev);

/**
* @brief 读取触摸数据
*
* @param data 读取到的触摸数据
* @return true 读取成功
* @return false 读取失败
*/
bool touch_read(touch_dev_t *data);

#endif /* TOUCH_SET_H */