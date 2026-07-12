#ifndef TIMER_SET_H
#define TIMER_SET_H

#include "driver/gptimer.h"

/**
 * @brief 初始化 GPTimer 硬件定时器
 *
 * 创建定时器、设置周期、注册回调函数，自动使能并启动
 *
 * @param period_us  定时周期（微秒），如 10000 = 10ms
 * @param callback   定时器到期回调函数
 * @param user_data  传递给回调函数的用户数据
 * @return 定时器句柄，失败返回 NULL
 */
gptimer_handle_t timer_init(uint32_t period_us,
                            gptimer_alarm_cb_t callback,
                            void *user_data);

#endif // TIMER_SET_H
