idf_component_register(SRCS "main.c"
                    REQUIRES "shtc3" "esp_event" "mock_wifi" "mock_flash"
                    INCLUDE_DIRS ".")