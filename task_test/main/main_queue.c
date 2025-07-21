//freeRTOS  队列的使用
#include "esp_log.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

QueueHandle_t xQueue = NULL;
int count;
void SendTask(void *param){
    while(1){
        if(xQueueSend(xQueue,&count,pdMS_TO_TICKS(100))==pdPASS){
            count++;
            ESP_LOGI("SendTask","send successed");
        }
        else{
            ESP_LOGI("SendTask","send feiled");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }   
}

void ReceiveTask(void *param){
    int Receive_count;
    while(1){
        if(xQueueReceive(xQueue,&Receive_count,pdMS_TO_TICKS(100))==pdPASS){
            ESP_LOGI("ReceiveTask","receive seccessed receive=%d",Receive_count);
        }
        else{
            ESP_LOGI("ReceiveTask","receive feiled");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void){
    xQueue = xQueueCreate(5,sizeof(int));
    if (xQueue!=NULL)
    {
        xTaskCreatePinnedToCore(SendTask,"Sender",2048,NULL,5,NULL,0);
        xTaskCreatePinnedToCore(ReceiveTask,"receiver", 2048,NULL,5,NULL,0);
    }

}

// xQueueSend( 
//     xQueue, 队列句柄
//     pvItemToQueue, 要发送的数据指针
//     xTicksToWait 等待超时时间
// )

// BaseType_t xQueueReceive( QueueHandle_t xQueue,队列句柄
//     void * const pvBuffer,接收数据的缓存区，QueueCreat()
//     TickType_t xTicksToWait 等待超时时间
//)
