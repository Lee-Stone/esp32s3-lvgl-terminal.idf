idf_component_register(SRCS "task_set.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ui_set uart_set lcd_set wifi_set data_set music_set xiaozhi_set esp_lvgl_games
                    REQUIRES espressif__esp_lvgl_adapter)