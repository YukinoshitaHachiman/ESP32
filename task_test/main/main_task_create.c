//freeRTOS 任务的创建
#include "stdio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct{
    int id;
    char *Name;
} Task_param;

TaskHandle_t TASKHAND_A = NULL;
TaskHandle_t TASKHAND_B = NULL;
int count;
void TaskA (void *param){
    while(1){
        Task_param *task_params = (Task_param *)param;
        ESP_LOGI("TaskA","id=%d  Name=%s",task_params->id,task_params->Name );
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskB(void *param){
    while(1){
        if(count == 5){
            ESP_LOGI("TaskB","Suspend TaskA");
            vTaskSuspend(TASKHAND_A);
        }
        else if(count == 10){
            ESP_LOGI("TaskB","Resume TaskA");
            vTaskResume(TASKHAND_A);
            count = 0;
        }
        count++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{   
    static Task_param task_param1={
        .id = 1,
        .Name = "Hello World"
    };
    xTaskCreatePinnedToCore(TaskA,"taskA",4096,&task_param1,4,&TASKHAND_A,0);
    xTaskCreatePinnedToCore(TaskB,"taskB",4096,NULL,4,&TASKHAND_B,0);
    
}

// BaseType_t xTaskCreatePinnedToCore( 
//     TaskFunction_t pxTaskCode,  -任务函数的指针
//     const char * const pcName,  -任务的名字，自定义，用于调试
//     const uint32_t usStackDepth,-任务堆栈大小，单位为字节
//     void * const pvParameters,   -传递给任务函数的参数指针，没有为NULL
//     UBaseType_t uxPriority,          -任务优先级 0~25 越大优先级越高
//     TaskHandle_t * const pxCreatedTask,  -任务句柄，没有为NULL
//     const BaseType_t xCoreID)            -指定CPU核心