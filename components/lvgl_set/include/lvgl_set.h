#ifndef LVGL_SET_H
#define LVGL_SET_H

#include "esp_lv_adapter.h"
#include "lvgl.h"
#include "lcd_set.h"
#include "touch_set.h"

/**
 * @brief 初始化 LVGL（适配器 + 显示 + 触摸）
 *
 * @param lcd_dev LCD 设备指针
 * @param touch_dev 触摸设备指针
 */
void lvgl_init(lcd_dev_t *lcd_dev, touch_dev_t *touch_dev);

#endif /* LVGL_SET_H */