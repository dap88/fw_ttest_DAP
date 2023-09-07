/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "Arduino.h"

extern "C"
{
    #include <stdio.h>
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
    //*** GLOBALS *** //
    //const char *ssid= "DAP"; // Wifi name
    //const char *pass= "USofDAP1"; // wifi Password
    //int retry_num= 0;
    TaskHandle_t helowHandle = NULL;
    static EventGroupHandle_t wifi_event_group; // Event group to contain status infromation
    static const char *TAG = "WIFI"; // task tag 

    #define WIFI_SUCCESS 1 << 0
    #define WIFI_FAILURE 1 << 1
    #define TCP_SUCCESS 1 << 0
    #define TCP_FAILURE 1 << 1
    #define MAX_FAILURES 10
    static int s_retry_num = 0; // Tracker
    

}

extern "C"
{
    // *** FUNTIONS *** //
    void app_main(void);
    void helow(void *arg);
    static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);
    //void wifi_connection();
    //static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,void *event_data);
}

//Arduino
#define LED 2



static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ESP_LOGI(TAG, "Connecting to AP...");
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		if (s_retry_num < MAX_FAILURES)
		{
			ESP_LOGI(TAG, "Reconnecting to AP...");
			esp_wifi_connect();
			s_retry_num++;
		} else {
			xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
		}
	}
}

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }

}

esp_err_t connect_wifi()
{
    int status = WIFI_FAILURE;
    //initialize the esp network interface
	ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg =WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /** EVENT LOOP CRAZINESS **/
	wifi_event_group = xEventGroupCreate();

    esp_event_handler_instance_t wifi_handler_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &wifi_handler_event_instance));

    esp_event_handler_instance_t got_ip_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &ip_event_handler,
                                                        NULL,
                                                        &got_ip_event_instance));

    /** START THE WIFI DRIVER **/
    //wifi_config_t wifi_config ={};

    const char ssid[]= "DAP"; // Wifi name
    const char pass[]= "USofDAP1"; // wifi Password
    wifi_config_t wifi_config;
    memset(&wifi_config,0,sizeof(wifi_config));
    strcpy((char*)wifi_config.sta.ssid,(const char*)ssid);
    strcpy((char*)wifi_config.sta.password,(const char*)pass);

    // set the wifi controller to be a station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );

    // set the wifi config
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

    // start the wifi driver
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "STA initialization complete");

    /** NOW WE WAIT **/
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
            WIFI_SUCCESS | WIFI_FAILURE,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_SUCCESS) {
        ESP_LOGI(TAG, "Connected to ap");
        status = WIFI_SUCCESS;
    } else if (bits & WIFI_FAILURE) {
        ESP_LOGI(TAG, "Failed to connect to ap");
        status = WIFI_FAILURE;
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        status = WIFI_FAILURE;
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
    vEventGroupDelete(wifi_event_group);
    return status;
}

void helow(void *arg){

    while(1){
        printf("epa colombia!\n");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

}

void app_main(void)
{   
    esp_err_t status = WIFI_FAILURE;

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    status =connect_wifi();
    //nvs_flash_init();
    xTaskCreate(helow,"helow", 4096, NULL, 10, &helowHandle);
    Serial.begin(115200);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    Serial.println("from arduino as a component\n");

}
