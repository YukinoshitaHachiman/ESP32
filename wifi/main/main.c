#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"

#define USER_SSID "Redmi Note 13 Pro"
#define USER_PASSWORD "11111111"

void wifi_event_handle(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data){
    if(event_base == WIFI_EVENT){
        switch(event_id){
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI("STA","wifi connect sucessful");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI("STA","wifi connect feiled,try again"); 
                esp_wifi_connect();
                vTaskDelay(pdMS_TO_TICKS(5000));
                break;
            default:break;
        }
    }
    else if(event_base == IP_EVENT){
        switch(event_id){
            case IP_EVENT_STA_GOT_IP:
                ESP_LOGI("IP","esp32 get ip address");
                break;
            default:break;
        }
    }
}
void app_main(void){

    nvs_flash_init();//nvs系统初始化，因为单片机会把ssid和password写到nvs系统中去
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_event_handle,NULL);
    esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,wifi_event_handle,NULL);

    wifi_config_t wifi_config={
        .sta.threshold.authmode = WIFI_AUTH_WPA2_PSK,
        .sta.pmf_cfg.capable = true,
        .sta.pmf_cfg.required = false,
        .sta.ssid = USER_SSID,
        .sta.password = USER_PASSWORD
    };
    
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA,&wifi_config);
    esp_wifi_start();

    return;
}
