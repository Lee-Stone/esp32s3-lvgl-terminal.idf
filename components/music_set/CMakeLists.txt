idf_component_register(SRCS "music_set.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_spi esp_driver_i2s esp-audio-player fatfs sdmmc task_set)
