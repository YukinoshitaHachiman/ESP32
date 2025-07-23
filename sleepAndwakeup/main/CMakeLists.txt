idf_component_register(SRCS  "hello_world_main.c" "touch_sensor.c"
                    PRIV_REQUIRES spi_flash driver
                    INCLUDE_DIRS "")
