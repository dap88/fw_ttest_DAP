#include "Arduino.h"

extern "C"
{
   #include <stdio.h> //for basic printf commands *
    #include <string.h> //for handling strings
    #include "freertos/FreeRTOS.h" //for delay,mutexs,semphrs rtos operations , set configuration required to run freeRTOS on ESP32*
    #include "freertos/task.h" // multitasking functionality*
    #include "esp_system.h" //esp_init funtions esp_err_t 
    #include "esp_wifi.h" //esp_wifi_init functions and wifi operations
    #include "esp_log.h" //for showing logs
    #include "esp_event.h" //for wifi event
    #include "nvs_flash.h" //non volatile storage
    #include "lwip/err.h" //light weight ip packets error handling
    #include "lwip/sys.h" //system applications for light weight ip apps 

}

extern "C"
{
   void app_main(void)
   void wifi_connection() 

}

extern "C"
{
    #define BLINK_GPIO 2
}

extern "C"
{
    const char *ssid= "DAP"; // Wifi name
    const char *pass= "USofDAP1"; // wifi Password
    int retry_num= 0;

}



static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,void *event_data){
    if(event_id == WIFI_EVENT_STA_START){
        printf("WiFi Connecting....\n");
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED){
        printf("WiFi conected\n");
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED){
        printf("WiFi lost connection\n");
        if(retry_num<5){
            esp_wifi_connect();
            retry_num++;
            printf("Retrying to Connect...\n");
            }
        else {
            printf("Restarting ESP32 in 5 seconds\n");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            fflush(stdout);
            esp_restart();
        }
    }
    else if (event_id == IP_EVENT_STA_GOT_IP)
    {
        printf("Wifi got IP...\n\n");
    }
}


void wifi_connection()
{
    esp_netif_init(); //network interdace initialization
    esp_event_loop_create_default(); // responisible for handling and dispatching events
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT(); // set up necessary data structs for wifi station interface
    esp_wifi_init (&wifi_initiation); // wifi initialised with defaul wifi_initation
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);//creating event handler register for wifi
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);//creating event handler register for ip event
    wifi_config_t wifi_configuration ={ //struct wifi_config_t var wifi_configuration
    .sta= {
        .ssid = "",
        .password= "", //const char of ssid and password
        }
    };
    strcpy((char*)wifi_configuration.sta.ssid, ssid);
    strcpy((char*)wifi_configuration.sta.password, pass); 
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_connect();
    printf( "wifi_init_softap finished. SSID:%s  password:%s",ssid,pass);

}   
    


void app_main(void)
{   
    nvs_flash_init();   
    wifi_connection();
   // xTaskCreate(helow,"helow", 2048, NULL, 5, NULL);
    //xTaskCreate(&blinky, "blinky", 512, NULL, 5, NULL);
}