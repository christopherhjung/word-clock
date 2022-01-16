/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi_connect.h"
#include "../config.h"

#define EXAMPLE_ESP_MAXIMUM_RETRY  5

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT        BIT0
#define WIFI_RECONNECTED_BIT      BIT1

static const char *WIFI_TAG = "wifi station";

int wifi_index = 0;
wifi_config_t wifi_config = { .sta = {} };
int conntection_trys = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT){
        if (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED) {
            if(event_id == WIFI_EVENT_STA_DISCONNECTED){
                xEventGroupClearBits( s_wifi_event_group, WIFI_CONNECTED_BIT );
                ESP_LOGI(WIFI_TAG,"connect to the AP fail");
            }

            if(conntection_trys > 20){
                esp_restart();
            }

            setup_wifi(&wifi_config, &wifi_index);
            if(wifi_index == -1){
                ESP_LOGE(WIFI_TAG, "Failed loading error");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                esp_restart();
            }

            conntection_trys++;
            ESP_LOGI(WIFI_TAG, "Try connect to. SSID=%s", wifi_config.sta.ssid);
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
            esp_wifi_connect();
        }
    }else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        conntection_trys = 0;
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(WIFI_TAG, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_RECONNECTED_BIT);
    }
}

void wifi_init_sta()
{
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    /*if(setup_wifi(&wifi_config, 0) != 0){
        ESP_LOGE(WIFI_TAG, "Failed loading error");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    }*/

    /* Setting a password implies station will connect to all security modes including WEP/WPA.
        * However these modes are deprecated and not advisable to be used. Incase your Access point
        * doesn't support WPA2, these mode can be enabled by commenting below line */

    if (strlen((char *)wifi_config.sta.password)) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    //ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(WIFI_TAG, "wifi_init_sta finished.");


    //ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    //ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    //vEventGroupDelete(s_wifi_event_group);
}

bool wifi_wait(){
    while(true){
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                               WIFI_CONNECTED_BIT | WIFI_RECONNECTED_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               portMAX_DELAY);

        if (bits & WIFI_CONNECTED_BIT) {
            if (bits & WIFI_RECONNECTED_BIT) {
                xEventGroupClearBits( s_wifi_event_group, WIFI_RECONNECTED_BIT );
                ESP_LOGI(WIFI_TAG, "connected to ap SSID:%s", wifi_config.sta.ssid);
            }
            break;
        }  else {
            ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
        }
    }

    return false;
}
/*
void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_LOGI(WIFI_TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
}
*/