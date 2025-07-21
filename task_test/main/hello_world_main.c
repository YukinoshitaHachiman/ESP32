/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
/************************************************************任务创建   ******************************************************************* */
// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// void taskA(void *param)
// {
//     while(1)
//     {
//         ESP_LOGI("main","Hello World!");
//         vTaskDelay(pdMS_TO_TICKS(500)); // Delay for 500ms
//     }
// }

// void app_main(void)
// {
//     xTaskCreatePinnedToCore(taskA, "helloworld", 2048, NULL, 5, NULL, 1); // Create taskA with no affinity
// }
/**************************************************************任务***************************************************************/



/************************************************************队列   ******************************************************************* */
// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "esp_log.h"
// #include <string.h>

// QueueHandle_t queue_handle = NULL; // Declare a queue handle


// typedef struct {
    
//  int value;
// }queue_data_t;

// void taskA(void *param)//队列接收数据
// {   
//     queue_data_t data; // Declare a variable to hold the received data
//     while(1)
//     {
//         if(pdTRUE == xQueueReceive(queue_handle, &data,100))
//         { 
//             ESP_LOGI("taskA","Received data: %d", data.value);
//         } 
//     }
// }

// void taskB(void *param)//每隔一秒发送数据
// {
//     queue_data_t data;
//     memset(&data,0,sizeof(queue_data_t)); // Initialize the data structure to zero
//     while(1)
//     {
//         xQueueSend(queue_handle, &data, 100); 
//         data.value ++;
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }


// }


// void app_main(void)
// {
//     queue_handle = xQueueCreate(10, sizeof(queue_data_t)); // Create a queue with 10 elements of type queue_data_t
//     xTaskCreatePinnedToCore(taskA, "taskA", 2048, NULL, 3, NULL, 1); // Create taskA with no affinity
//     xTaskCreatePinnedToCore(taskB, "taskB", 2048, NULL, 3, NULL, 1);
// }
/************************************************************队列   ******************************************************************* */


/************************************************************信号量   ******************************************************************* */

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/semphr.h" // Include the FreeRTOS semaphore header
// #include "esp_log.h"
// #include <string.h>
// SemaphoreHandle_t bin_sem ; // Declare a semaphore handle

// void taskA(void *param)
// {   
//     while(1)
//     {
//         xSemaphoreGive(bin_sem);
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1000ms
//     }
// }

// void taskB(void *param)
// {
//     while(1)
//     {
//         if(pdTRUE == xSemaphoreTake(bin_sem,portMAX_DELAY)) // Wait for the semaphore to be available
//         { 
//             ESP_LOGI("taskB","Semaphore taken by taskB");
//         } 
//     }
// }

// void app_main(void)

// {   
//     bin_sem = xSemaphoreCreateBinary(); // Create a binary semaphore
//     xTaskCreatePinnedToCore(taskA, "taskA", 2048, NULL, 3, NULL, 1); // Create taskA with no affinity
//     xTaskCreatePinnedToCore(taskB, "taskB", 2048, NULL, 4, NULL, 1); // Create taskB with no affinity
// }

/************************************************************信号量   ******************************************************************* */

/************************************************************互斥锁   ******************************************************************* */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h" // Include the FreeRTOS semaphore header
#include "esp_log.h"
#include <string.h>
#include "esp32_s3_szp.h" // Include the QMI8658 header file

SemaphoreHandle_t qmi8658_mutex ; // Declare a semaphore handle
t_sQMI8658 QMI8658; // 定义QMI8658结构体变量


void taskA(void *param)
{   
    while(1)
    {   
        vTaskDelay(pdMS_TO_TICKS(500));
        if(xSemaphoreTake(qmi8658_mutex,portMAX_DELAY)) 
        {
            qmi8658_fetch_angleFromAcc(&QMI8658);
            ESP_LOGI("taskA","taskA--->x=%f,y=%f,z=%f",QMI8658.AngleX,QMI8658.AngleY,QMI8658.AngleZ);
        
            xSemaphoreGive(qmi8658_mutex);
        }   
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void taskB(void *param)
{
    while(1)
    {   
       
        vTaskDelay(pdMS_TO_TICKS(500));
        if(xSemaphoreTake(qmi8658_mutex,portMAX_DELAY)) 
        {
            qmi8658_fetch_angleFromAcc(&QMI8658);
            ESP_LOGI("taskB","taskB--->x=%f,y=%f,z=%f",QMI8658.AngleX,QMI8658.AngleY,QMI8658.AngleZ);
            xSemaphoreGive(qmi8658_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)

{   bsp_i2c_init();
    qmi8658_init();
    qmi8658_mutex = xSemaphoreCreateMutex(); // Create a binary semaphore
    xTaskCreatePinnedToCore(taskA, "taskA", 2048, NULL, 5, NULL, 1); // Create taskA with no affinity
    xTaskCreatePinnedToCore(taskB, "taskB", 2048, NULL, 4, NULL, 1); // Create taskB with no affinity
}
/************************************************************互斥锁   ******************************************************************* */

  
