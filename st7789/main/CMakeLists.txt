idf_component_register(SRCS  "hello_world_main.c" "i2c_master.c" "pca9557.c" 
                    PRIV_REQUIRES spi_flash driver esp_lcd
                    INCLUDE_DIRS "")
