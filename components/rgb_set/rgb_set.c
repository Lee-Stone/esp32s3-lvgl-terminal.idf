#include <stdio.h>
#include "rgb_set.h"

// void rgb_init(void)
// {
//     // 配置 R、G、B 三个引脚为 GPIO 输出模式
//     gpio_config_t io_conf = {
//         .pin_bit_mask = (1ULL << RGB_R_PIN) | (1ULL << RGB_G_PIN) | (1ULL << RGB_B_PIN),
//         .mode = GPIO_MODE_OUTPUT,
//         .pull_up_en = GPIO_PULLUP_DISABLE,
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .intr_type = GPIO_INTR_DISABLE,
//     };
//     gpio_config(&io_conf);

//     // 默认关闭 LED
//     rgb_set_color(RGB_COLOR_OFF);
// }

void rgb_init(void)
{
    // 配置 LEDC 定时器
    ledc_timer_config_t timer_conf = {
        .speed_mode      = RGB_LEDC_MODE,
        .duty_resolution = RGB_LEDC_DUTY_RES,
        .timer_num       = RGB_LEDC_TIMER,
        .freq_hz         = RGB_LEDC_FREQUENCY,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_conf);

    // 配置红色通道（初始 duty=RGB_DUTY_MAX 输出高电平，LED 灭）
    ledc_channel_config_t r_conf = {
        .gpio_num   = RGB_R_PIN,
        .speed_mode = RGB_LEDC_MODE,
        .channel    = RGB_LEDC_CHANNEL_R,
        .timer_sel  = RGB_LEDC_TIMER,
        .duty       = RGB_DUTY_MAX,
        .hpoint     = 0,
    };
    ledc_channel_config(&r_conf);

    // 配置绿色通道
    ledc_channel_config_t g_conf = {
        .gpio_num   = RGB_G_PIN,
        .speed_mode = RGB_LEDC_MODE,
        .channel    = RGB_LEDC_CHANNEL_G,
        .timer_sel  = RGB_LEDC_TIMER,
        .duty       = RGB_DUTY_MAX,
        .hpoint     = 0,
    };
    ledc_channel_config(&g_conf);

    // 配置蓝色通道
    ledc_channel_config_t b_conf = {
        .gpio_num   = RGB_B_PIN,
        .speed_mode = RGB_LEDC_MODE,
        .channel    = RGB_LEDC_CHANNEL_B,
        .timer_sel  = RGB_LEDC_TIMER,
        .duty       = RGB_DUTY_MAX,
        .hpoint     = 0,
    };
    ledc_channel_config(&b_conf);
}

// void rgb_set_color(uint8_t color)
// {
//     // 低电平点亮：color 位为 1 时输出低电平（点亮），为 0 时输出高电平（熄灭）
//     gpio_set_level(RGB_R_PIN, (color & 0x04) ? 0 : 1);
//     gpio_set_level(RGB_G_PIN, (color & 0x02) ? 0 : 1);
//     gpio_set_level(RGB_B_PIN, (color & 0x01) ? 0 : 1);
// }

void rgb_set_color(uint16_t r_duty, uint16_t g_duty, uint16_t b_duty)
{
    // 限制占空比范围（0~1023）
    if (r_duty > RGB_DUTY_MAX) r_duty = RGB_DUTY_MAX;
    if (g_duty > RGB_DUTY_MAX) g_duty = RGB_DUTY_MAX;
    if (b_duty > RGB_DUTY_MAX) b_duty = RGB_DUTY_MAX;

    // 低电平点亮：LEDC duty 控制高电平时间，需反转
    // 用户传入的 duty 值越大 → LED 越亮 → 实际高电平时间越短
    ledc_set_duty(RGB_LEDC_MODE, RGB_LEDC_CHANNEL_R, RGB_DUTY_MAX - r_duty);
    ledc_update_duty(RGB_LEDC_MODE, RGB_LEDC_CHANNEL_R);

    ledc_set_duty(RGB_LEDC_MODE, RGB_LEDC_CHANNEL_G, RGB_DUTY_MAX - g_duty);
    ledc_update_duty(RGB_LEDC_MODE, RGB_LEDC_CHANNEL_G);

    ledc_set_duty(RGB_LEDC_MODE, RGB_LEDC_CHANNEL_B, RGB_DUTY_MAX - b_duty);
    ledc_update_duty(RGB_LEDC_MODE, RGB_LEDC_CHANNEL_B);
}