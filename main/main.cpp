

#include "network/wifi_connect.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <netdb.h>
#include <sys/socket.h>

#include "terminal/terminal.h"
#include "config.h"
#include "core.h"

#include "network/client_connect.h"

#define SIIAM_WIFI_SSID "WLAN-396851"
#define SIIAM_WIFI_PASSWORD "70235564365384924196"
#define SIIAM_HOST "192.168.2.176"
#define SIIAM_TOKEN "66687aadf862bd776c8fc18b8e9f8e20089714856ee233b3902a591d0d5f2925"

int myPutChar(int cha){
    return cha;
}

void run(){
    ESP_ERROR_CHECK (nvs_flash_init());
    ESP_ERROR_CHECK (esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    mount();
    loadConfig();

    ESP_LOGI("wifi", "ESP_WIFI_MODE_STA" ) ;
    setToken(SIIAM_TOKEN);
    wifi_init_sta ( SIIAM_WIFI_SSID , SIIAM_WIFI_PASSWORD ) ;

    //esp_log_set_putchar(myPutChar);
    //runTerminal();
    runControlLink(SIIAM_HOST);
}

extern "C"{
    void app_main() {
        run();
    }
}
