#ifndef KEY_SET_H
#define KEY_SET_H

#include "driver/gpio.h"

// 按键引脚定义
#define KEY_1_PIN  GPIO_NUM_2   // 按键1
#define KEY_2_PIN  GPIO_NUM_45  // 按键2
#define KEY_3_PIN  GPIO_NUM_46  // 按键3

/**
 * @brief 初始化按键
 * 
 * 将三个按键引脚配置为 GPIO 输入模式，启用内部上拉电阻
 */
void key_init(void);

/**
 * @brief 读取按键值
 * 
 * 检测按键按下并等待松开后返回按键编号
 * @return 0 表示无按键按下，1/2/3 表示按键1/2/3 被按下
 */
uint8_t key_value_read(void);

#endif /* KEY_SET_H */
