idf_component_register(SRCS "uart_set.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_uart esp_driver_gpio task_set)
