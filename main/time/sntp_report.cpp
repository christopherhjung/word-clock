//
// Created by Christopher Jung on 18.04.24.
//

#include "sntp_report.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/apps/sntp.h"

static const char *TAG = "sntp_report";

static void sntp_report_task(void *arg)
{
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

    while (1) {
        // update 'now' variable with current time
        time(&now);
        localtime_r(&now, &timeinfo);

        if (timeinfo.tm_year > 100) {
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
        }

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void sntp_report_init()
{
    xTaskCreate(sntp_report_task, "sntp_report_task", 2048, NULL, 10, NULL);
}
