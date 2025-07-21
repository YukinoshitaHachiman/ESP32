#include "uart_init.h"
#include "esp_log.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
u_int8_t uart_buffer[1024];
QueueHandle_t uart_queue;
uart_event_t uart_evt;
//更改模块的设置后，掉电一段时间后设置会回复，所以写了一个在接收前自动发送AT指令进行设置
char port_chance[]="$PCAS01,5*19\r\n";
char rec_chance[]="$PCAS03,1,0,0,0,0,0,0,0,0,0,,,1,0*02\r\n";

void uart_chance_mode(void){
    uart_write_bytes(USER_UART,&port_chance,sizeof(port_chance));
    uart_write_bytes(USER_UART,&rec_chance,sizeof(rec_chance));
}

void uart_task(void){
    if(pdTRUE==xQueueReceive(uart_queue,&uart_evt,portMAX_DELAY)){    
        switch(uart_evt.type){
            case UART_DATA://接收到串口数据
                uart_read_bytes(USER_UART,uart_buffer,uart_evt.size,portMAX_DELAY);
                //uart_buffer[uart_evt.size]='1';
                //ESP_LOGI("UART","串口数据为:%s",uart_buffer);
                break;
            case UART_BUFFER_FULL: //缓存区满
                break;
            case UART_FIFO_OVF://fifo溢出
                break;
            default:break;
            }
        }
}

void gcc_get(void) {
    // 模拟串口接收到的数据
    //static u_int8_t uart_buffer_test[1024] = "$GNGGA,235959.999,1145.1400,N,1919.8100,W,0,00,25.5,0,0,0,0,0,0*64\r\n\0";

    // 使用 char 指针处理字符串
    char *str = (char *)uart_buffer;
    char *token;
    int comma_count = 0;
    // 用来存储我们需要的数据
    char result_time[16];
    char result_llll[16];
    char result_yyyy[16];
    // 使用 strtok 逐个分割
    token = strtok(str, ",");
    while (token != NULL) {
        if (comma_count == 1) {
            strncpy(result_time, token, sizeof(result_time) - 1);
        }
        else if(comma_count == 2){
            strncpy(result_llll, token, sizeof(result_llll) - 1);
        }
        else if(comma_count == 4){
            strncpy(result_yyyy, token, sizeof(result_yyyy) - 1);
        }
        token = strtok(NULL, ",");
        comma_count++;
    }
    // 输出提取结果
    int time = atoi(result_time);
    int s = time%100;
    int m = time%10000-s*100;
    int h = (time-m*100-s)/10000;
    printf("提取的数据为:time= %d.%d.%d\n",h,m,s);
    printf("提取的数据为:经度= %s\n", result_llll);
    printf("提取的数据为:纬度= %s\n", result_yyyy);
}

void uart_init(void){
    uart_config_t uart_config = {
        .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .parity    = UART_PARITY_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
        .stop_bits = UART_STOP_BITS_1, 
    };
    uart_param_config(USER_UART, &uart_config); //配置串口参数
    uart_set_pin(USER_UART, GPIO_NUM_11, GPIO_NUM_10, -1, -1); //设置串口引脚
    uart_driver_install(USER_UART,4096,2048,20,&uart_queue,0);//安装驱动
}