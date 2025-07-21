//freeRTOS事件组
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#define NUM_BIT0 0x00000001
#define NUM_BIT1 0x00000002

EventGroupHandle_t event_g;

// TaskHandle_t TaskA = NULL;
// TaskHandle_t TaskB = NULL;

void TaskA(void *param){
    while(1){
        xEventGroupSetBits(event_g, NUM_BIT0);
        ESP_LOGI("TaskA","BIT0 set");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskB(void *param){
    while(1){
        xEventGroupSetBits(event_g, NUM_BIT1);
        ESP_LOGI("TaskB","BIT1 set");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

EventBits_t ev;
void event_receiver(void *param){
    while(1){
        ev = xEventGroupWaitBits(event_g,NUM_BIT0 | NUM_BIT1,pdFALSE,pdFALSE,portMAX_DELAY);
        if((ev & ((NUM_BIT0 | NUM_BIT1))) == (NUM_BIT0 | NUM_BIT1)){
            ESP_LOGI("event_receiver","both A and B has been setted to 1,clear them");
            xEventGroupClearBits(event_g,NUM_BIT0 | NUM_BIT1);
        }
    }
}

void app_main(void){
    event_g = xEventGroupCreate();
    xTaskCreatePinnedToCore(TaskA,"TaskA",2048,NULL,3,NULL,1);
    xTaskCreatePinnedToCore(TaskB,"TaskB",2048,NULL,3,NULL,1);
    xTaskCreatePinnedToCore(event_receiver,"event group",2048,NULL,3,NULL,1);
}