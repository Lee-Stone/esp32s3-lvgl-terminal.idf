idf_component_register(SRCS "lvgl_set.c"
                    INCLUDE_DIRS "include"
                    REQUIRES lcd_set touch_set
                    REQUIRES espressif__esp_lvgl_adapter)
