#include <stdio.h>
#include "uart_set.h"
#include "task_set.h"
#include <string.h>

void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_param_config(UART_PORT_NUM, &uart_config);

    uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE, 0, 0, NULL, 0);
}

void uart_task(void *pvParameters)
{
    while (1) {
        if (uart_read_bytes(UART_PORT_NUM, app.uart.rx_data, sizeof(app.uart.rx_data) - 1, 20 / portTICK_PERIOD_MS) > 0) {
            if (esp_lv_adapter_lock(-1) == ESP_OK) {
                lv_textarea_add_text(ui_TextAreaRX, (const char *)app.uart.rx_data);
                esp_lv_adapter_unlock();
            }

            memset(app.uart.rx_data, 0, sizeof(app.uart.rx_data));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}