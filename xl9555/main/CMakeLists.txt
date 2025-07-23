idf_component_register(SRCS "xl9555.c" "hello_world_main.c" "i2c_master.c"
                    PRIV_REQUIRES spi_flash driver
                    INCLUDE_DIRS "")
