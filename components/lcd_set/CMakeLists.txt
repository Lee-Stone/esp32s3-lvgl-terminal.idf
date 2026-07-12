idf_component_register(SRCS "lcd_set.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_lcd esp_driver_gpio esp_driver_spi)
