#ifndef TOUCH_SET_H
#define TOUCH_SET_H

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_lcd_touch_ft5x06.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_io_interface.h"

// FT6336(FT5x06 系列) I2C 触摸引脚定义（对应原理图 Schematic1_1 ESP32S3智能终端）
#define TOUCH_SDA_PIN   GPIO_NUM_3
#define TOUCH_SCL_PIN   GPIO_NUM_18
#define TOUCH_RST_PIN   GPIO_NUM_8
#define TOUCH_INT_PIN   GPIO_NUM_46   // 中断引脚

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
