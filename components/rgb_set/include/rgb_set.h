#ifndef RGB_SET_H
#define RGB_SET_H

#include "driver/gpio.h"
#include "driver/ledc.h"

#define RGB_R_PIN  GPIO_NUM_21
#define RGB_G_PIN  GPIO_NUM_47
#define RGB_B_PIN  GPIO_NUM_48

// LEDC PWM 配置
#define RGB_LEDC_TIMER       LEDC_TIMER_0      // 定时器
#define RGB_LEDC_MODE        LEDC_LOW_SPEED_MODE // 低速模式
#define RGB_LEDC_CHANNEL_R   LEDC_CHANNEL_0     // 红色通道
#define RGB_LEDC_CHANNEL_G   LEDC_CHANNEL_1     // 绿色通道
#define RGB_LEDC_CHANNEL_B   LEDC_CHANNEL_2     // 蓝色通道
#define RGB_LEDC_DUTY_RES    LEDC_TIMER_10_BIT  // 10 位分辨率（0~1023）
#define RGB_LEDC_FREQUENCY   5000               // PWM 频率 5kHz
#define RGB_DUTY_MAX         1023               // 最大占空比值

// 预定义颜色（低电平点亮，1 = 亮）
#define RGB_COLOR_RED      0x04  // R=1, G=0, B=0  0000 0100
#define RGB_COLOR_GREEN    0x02  // R=0, G=1, B=0  0000 0010
#define RGB_COLOR_BLUE     0x01  // R=0, G=0, B=1  0000 0001
#define RGB_COLOR_YELLOW   (RGB_COLOR_RED | RGB_COLOR_GREEN)
#define RGB_COLOR_CYAN     (RGB_COLOR_GREEN | RGB_COLOR_BLUE)
#define RGB_COLOR_MAGENTA  (RGB_COLOR_RED | RGB_COLOR_BLUE)
#define RGB_COLOR_WHITE    (RGB_COLOR_RED | RGB_COLOR_GREEN | RGB_COLOR_BLUE)
#define RGB_COLOR_OFF      0x00  // 全部关闭

/**
 * @brief 初始化 RGB LED
 * 
 * 将 R、G、B 三个引脚配置为 GPIO 输出模式，并默认关闭 LED
 */
void rgb_init(void);

// /**
//  * @brief 设置 RGB LED 颜色
//  * 
//  * @param color 颜色值，使用 RGB_COLOR_xxx 宏定义，如 RGB_COLOR_RED
//  */
// void rgb_set_color(uint8_t color);

/**
 * @brief 设置 RGB LED 颜色深度
 *
 * @param r_duty 红色占空比（0~1023），0 为全灭，1023 为最亮
 * @param g_duty 绿色占空比（0~1023），0 为全灭，1023 为最亮
 * @param b_duty 蓝色占空比（0~1023），0 为全灭，1023 为最亮
 */
void rgb_set_color(uint16_t r_duty, uint16_t g_duty, uint16_t b_duty);


#endif

