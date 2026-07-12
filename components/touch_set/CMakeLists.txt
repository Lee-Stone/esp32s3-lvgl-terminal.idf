idf_component_register(SRCS "touch_set.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_gpio esp_driver_softspi atanisoft__esp_lcd_touch_xpt2046)
