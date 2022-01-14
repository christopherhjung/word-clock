

#include "network/wifi_connect.h"

#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "terminal/terminal.h"
#include "config.h"

#include "network/client_connect.h"

#include "driver/adc.h"


int myPutChar(int cha){
    return cha;
}

void run(){
    ESP_ERROR_CHECK (nvs_flash_init());
    ESP_ERROR_CHECK (esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    init_config();

    adc_config_t adc_config;
    adc_config.mode = ADC_READ_TOUT_MODE;
    adc_config.clk_div = 8;
    ESP_ERROR_CHECK(adc_init(&adc_config));

    mount();
    listFiles(".");
    listFiles("/spiffs");
    char* content = loadFile("/spiffs/config.json");
    if(content != NULL){
        printf("%s", content);
    }
    loadConfig();


    ESP_LOGI("wifi", "ESP_WIFI_MODE_STA" ) ;
    wifi_init_sta () ;

    //esp_log_set_putchar(myPutChar);
    //runTerminal();
    runControlLink();
}

extern "C"{
    void app_main() {
        run();
    }
}
