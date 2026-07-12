#include <stdio.h>
#include "timer_set.h"

gptimer_handle_t timer_init(uint32_t period_us,
                            gptimer_alarm_cb_t callback,
                            void *user_data)
{
    // 1. 创建定时器
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000,  // 1MHz，即 1μs 计数一次
    };
    gptimer_handle_t gptimer = NULL;
    gptimer_new_timer(&timer_config, &gptimer);

    // 2. 设置报警参数（自动重载）
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = period_us,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(gptimer, &alarm_config);

    // 3. 注册回调函数
    gptimer_event_callbacks_t cb_config = {
        .on_alarm = callback,
    };
    gptimer_register_event_callbacks(gptimer, &cb_config, user_data);

    // 4. 使能并启动定时器
    gptimer_enable(gptimer);
    gptimer_start(gptimer);

    return gptimer;
}