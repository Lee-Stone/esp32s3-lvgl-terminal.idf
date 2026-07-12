#include <stdio.h>
#include "lvgl_set.h"

void lvgl_init(lcd_dev_t *lcd_dev, touch_dev_t *touch_dev)
{
    // 1. 初始化 LVGL 适配器
    esp_lv_adapter_config_t cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    esp_lv_adapter_init(&cfg);

    // 2. 注册显示设备（SPI 接口 + PSRAM）
    esp_lv_adapter_display_config_t disp_cfg =
        ESP_LV_ADAPTER_DISPLAY_SPI_WITH_PSRAM_DEFAULT_CONFIG(
            lcd_dev->panel, lcd_dev->panel_io,
            LCD_WIDTH, LCD_HEIGHT, ESP_LV_ADAPTER_ROTATE_0);
    lv_display_t *disp = esp_lv_adapter_register_display(&disp_cfg);

    // 3. 注册触摸输入设备
    esp_lv_adapter_touch_config_t touch_cfg =
        ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, touch_dev->handle);
    esp_lv_adapter_register_touch(&touch_cfg);

    // 4. 启动适配器任务
    esp_lv_adapter_start();
}