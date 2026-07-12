#include <stdio.h>
#include "touch_set.h"
#include <string.h>

esp_err_t esp_lcd_new_panel_io_softspi(const esp_lcd_panel_io_spi_config_t *io_config, esp_lcd_panel_io_handle_t *ret_io);

static esp_lcd_touch_handle_t touch_handle = NULL;

void touch_init(touch_dev_t *dev)
{
    // 软件 SPI 总线
    softspi_bus_config_t bus_cfg = {
        .sclk_io_num = TOUCH_CLK_PIN,
        .mosi_io_num = TOUCH_DIN_PIN,
        .miso_io_num = TOUCH_DO_PIN,
    };
    softspi_bus_initialize(&bus_cfg);

    // 创建 SPI 面板 IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(TOUCH_CS_PIN);
    esp_lcd_new_panel_io_softspi(&io_config, &io_handle);

    // XPT2046 触摸配置
    esp_lcd_touch_config_t touch_cfg = {
        .x_max = TOUCH_HEIGHT,
        .y_max = TOUCH_WIDTH,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = TOUCH_IRQ_PIN,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 1,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    esp_lcd_touch_new_spi_xpt2046(io_handle, &touch_cfg, &touch_handle);
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

static esp_err_t rx_param(esp_lcd_panel_io_t *io, int lcd_cmd, void *param, size_t param_size)
{
    int cs = *(int *)(io + 1);
    uint8_t cmd = (uint8_t)lcd_cmd, rx[8];
    softspi_transfer(cs, &cmd, rx, param_size + 1);
    memcpy(param, rx + 1, param_size);
    return ESP_OK;
}
static esp_err_t tx_param(esp_lcd_panel_io_t *io, int cmd, const void *p, size_t s)
{ return ESP_ERR_NOT_SUPPORTED; }
static esp_err_t tx_color(esp_lcd_panel_io_t *io, int cmd, const void *c, size_t s)
{ return ESP_ERR_NOT_SUPPORTED; }
static esp_err_t del(esp_lcd_panel_io_t *io)
{ free(io); return ESP_OK; }

esp_err_t esp_lcd_new_panel_io_softspi(const esp_lcd_panel_io_spi_config_t *io_config, esp_lcd_panel_io_handle_t *ret_io)
{
    int cs = io_config->cs_gpio_num;
    gpio_config(&(gpio_config_t){
        .pin_bit_mask = (1ULL << cs),
        .mode = GPIO_MODE_OUTPUT, .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE, .intr_type = GPIO_INTR_DISABLE,
    });
    gpio_set_level(cs, 1);
    esp_lcd_panel_io_t *io = calloc(1, sizeof(esp_lcd_panel_io_t) + sizeof(int));
    *(int *)(io + 1) = cs;
    io->rx_param = rx_param;
    io->tx_param = tx_param;
    io->tx_color = tx_color;
    io->del      = del;
    *ret_io = io;
    return ESP_OK;
}