#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "stdio.h"
#include "time.h"
#include "math.h"
SemaphoreHandle_t xSemphr_t;
int count[10]={1,2,3,4,5,6,7,8,9,10};
int temp;
//任务A让整个数组一秒钟左移一位
void TaskA(void *param){
    while(1){
        if(xSemaphoreTake(xSemphr_t,portMAX_DELAY)==pdPASS){
            temp = count[0];
            for(int i=0;i<9;i++){
                count[i]=count[i+1];
            }
            count[9]=temp;
        }
        xSemaphoreGive(xSemphr_t);
        //ESP_LOGI("TaskA","%d",count[0]);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
//任务B每两秒尝试读取数组首位
void TaskB(void *param){
    while(1){
        if(xSemaphoreTake(xSemphr_t,portMAX_DELAY)==pdPASS){
            ESP_LOGI("TaskB","%d",count[0]);
        }
        xSemaphoreGive(xSemphr_t);
        //srand(time(NULL));
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void app_main(void)
{
    xSemphr_t = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(TaskA,"TaskA",2048,NULL,5,NULL,0);
    xTaskCreatePinnedToCore(TaskB,"TaskB",2048,NULL,4,NULL,0);
}
