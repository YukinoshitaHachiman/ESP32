#include <stdio.h>
#include "esp_log.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart_init.h"

#define TAG "uart_main"




void app_main(void){
   
    //uart_init();
    //uart_chance_mode();
    
    while(1){
        //uart_task();
        //gcc_get();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
