#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"
#include "uart_init.h"

#define USER_UART UART_NUM_2
extern u_int8_t uart_buffer[1024];
extern QueueHandle_t uart_queue;
extern uart_event_t uart_evt;

void uart_init(void);
void uart_task(void);
void uart_chance_mode(void);
void gcc_get(void);
