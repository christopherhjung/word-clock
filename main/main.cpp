
#include "network/wifi_connect.h"

#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "config.h"
#include "time/sntp_report.h"
#include "neopixel/display.h"
#include "neopixel/renderer.h"
#include "neopixel/dimmer.h"
#include "api/api.h"

#include "driver/adc.h"

static const char *TAG = "main";

static void sntp_setup(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

void run(){
    ESP_ERROR_CHECK (nvs_flash_init());
    ESP_ERROR_CHECK (esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    config_init();
    mount();
    config_load();

    adc_config_t adc_config;
    adc_config.mode = ADC_READ_TOUT_MODE;
    adc_config.clk_div = 8;
    ESP_ERROR_CHECK(adc_init(&adc_config));

    listFiles(".");
    listFiles("/spiffs");
    char* content = loadFile("/spiffs/config.json");
    if(content != NULL){
        printf("%s", content);
    }

    ESP_LOGI("wifi", "ESP_WIFI_MODE_STA" ) ;
    wifi_init_sta();
    sntp_setup();
    sntp_report_init();

    display_init(114);
    dimmer_init();
    renderer_init();

    rest_init();
}

extern "C"{
    void app_main() {
        run();
    }
}
