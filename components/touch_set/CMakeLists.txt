idf_component_register(SRCS "touch_set.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_gpio esp_driver_i2c esp_lcd espressif__esp_lcd_touch_ft5x06)
