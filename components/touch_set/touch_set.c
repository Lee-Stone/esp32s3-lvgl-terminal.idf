#include <stdio.h>
#include "touch_set.h"
#include <string.h>

static i2c_master_bus_handle_t i2c_bus_handle = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

void touch_init(touch_dev_t *dev)
{
    // I2C 主机总线
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = TOUCH_SDA_PIN,
        .scl_io_num = TOUCH_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_new_master_bus(&bus_cfg, &i2c_bus_handle);

    // 创建 I2C 面板 IO（FT6336 地址 0x38）
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
    esp_lcd_new_panel_io_i2c(i2c_bus_handle, &io_config, &io_handle);

    // FT6336(FT5x06) 触摸配置
    esp_lcd_touch_config_t touch_cfg = {
        .x_max = TOUCH_HEIGHT,
        .y_max = TOUCH_WIDTH,
        .rst_gpio_num = TOUCH_RST_PIN,
        .int_gpio_num = TOUCH_INT_PIN,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 1,
            .mirror_x = 0,
            .mirror_y = 1,   // 屏幕X取自raw Y且方向相反，交换前先镜像Y(量程=y_max=320)
        },
    };

    esp_lcd_touch_new_i2c_ft5x06(io_handle, &touch_cfg, &touch_handle);
    dev->handle = touch_handle;
}

bool touch_read(touch_dev_t *data)
{
    data->pressed = false;
    if (touch_handle == NULL) return false;

    esp_lcd_touch_read_data(touch_handle);

    uint16_t x[1], y[1], strength[1];
    uint8_t point_num = 0;
    bool touched = esp_lcd_touch_get_coordinates(touch_handle, x, y, strength, &point_num, 1);

    if (touched && point_num > 0) {
        data->x = x[0];
        data->y = y[0];
        data->pressed = true;
        return true;
    }
    return false;
}
