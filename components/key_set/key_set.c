#include <stdio.h>
#include "key_set.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void key_init(void)
{
    // 配置三个按键引脚为 GPIO 输入模式，启用内部上拉电阻
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << KEY_1_PIN) | (1ULL << KEY_2_PIN) | (1ULL << KEY_3_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
}

uint8_t key_value_read(void)
{
    // 按键1按下
    if (gpio_get_level(KEY_1_PIN) == 0) {
        // 等待按键松开
        while (gpio_get_level(KEY_1_PIN) == 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        return 1;
    }
    // 按键2按下
    if (gpio_get_level(KEY_2_PIN) == 0) {
        while (gpio_get_level(KEY_2_PIN) == 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        return 2;
    }
    // 按键3按下
    if (gpio_get_level(KEY_3_PIN) == 0) {
        while (gpio_get_level(KEY_3_PIN) == 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        return 3;
    }
    return 0;
}