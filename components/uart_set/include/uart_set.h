#ifndef UART_SET_H
#define UART_SET_H

#include "driver/uart.h"
#include "driver/gpio.h"

// UART0 配置（TX0 = GPIO43, RX0 = GPIO44）
#define UART_PORT_NUM    UART_NUM_0
#define UART_BAUD_RATE   115200
#define UART_TX_PIN      GPIO_NUM_43
#define UART_RX_PIN      GPIO_NUM_44
#define UART_BUF_SIZE    1024

/**
 * @brief 初始化 UART 串口（UART0，115200，8N1）
 */
void uart_init(void);

/**
* @brief UART 任务函数
* @param pvParameters 任务参数
*/
void uart_task(void *pvParameters);

#endif /* UART_SET_H */