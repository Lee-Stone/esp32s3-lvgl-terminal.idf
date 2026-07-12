idf_component_register(SRCS "wifi_set.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi esp_netif nvs_flash task_set)
