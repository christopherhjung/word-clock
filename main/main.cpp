#include "network/client_connect.h"
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
#include "neopixel/neopixel.h"

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
    wifi_init_sta ( "WLAN-396851" , "70235564365384924196" ) ;

    esp_log_set_putchar(myPutChar);
    runTerminal();
    runControlLink();
}

extern "C"{
    void app_main() {
        run();
    }
}
