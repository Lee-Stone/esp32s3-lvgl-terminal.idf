idf_component_register(SRCS "xiaozhi_set.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_i2s esp-audio-player esp_event esp_netif 
                    REQUIRES espressif__mcp-c-sdk espressif__esp_xiaozhi esphome__micro-opus
                    REQUIRES task_set)