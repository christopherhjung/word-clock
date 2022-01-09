

#include "commands.h"
#include "../core.h"
#include "../protocol.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_ota_ops.h"
#include "../config.h"

#define EXAMPLE_SERVER_IP   CONFIG_SERVER_IP
#define EXAMPLE_SERVER_PORT CONFIG_SERVER_PORT
#define EXAMPLE_FILENAME CONFIG_EXAMPLE_FILENAME
#define BUFFSIZE 1500
#define TEXT_BUFFSIZE 1024

static const char *TAG = "ota";

static void __attribute__((noreturn)) task_fatal_error()
{
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    while (1) {
        ;
    }
}

esp_ota_handle_t update_handle = 0;
const esp_partition_t *update_partition = NULL;
uint32_t binarySize;
uint32_t writtenSize;
float lastPercent;
void io_siiam$update_begin$1_0_0(uint8_t length, uint8_t* frame){
    uint32_t size = nextUInt32(&frame);
    ESP_LOGI(TAG, "Starting OTA of size %d", size);

    const esp_partition_t *to_boot;

    to_boot = esp_ota_get_boot_partition();
    if (!to_boot) {
        ESP_LOGI(TAG, "no OTA boot partition");
        to_boot = esp_ota_get_running_partition();
        if (!to_boot) {
            ESP_LOGE(TAG, "ERROR: Fail to get running partition");
            task_fatal_error();
        }
    }

    update_partition = esp_ota_get_next_update_partition(NULL);
    if(update_partition == NULL){
        ESP_LOGE(TAG, "ERROR: Fail to get update partition");
        task_fatal_error();
    }

    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);

    esp_err_t err = esp_ota_begin(update_partition, size, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
        task_fatal_error();
    }
    ESP_LOGI(TAG, "esp_ota_begin succeeded");

    uint8_t ota_num = get_ota_partition_count();
    uint8_t update_ota_num = update_partition->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_0;

    ESP_LOGI(TAG, "Totoal OTA number %d update to %d part", ota_num, update_ota_num);
}

void io_siiam$update_write$1_0_0(uint8_t length, uint8_t* frame){
    esp_err_t err = esp_ota_write( update_handle, (const void *)frame, length);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
        task_fatal_error();
    }

    writtenSize += length;
    ESP_LOGE(TAG, "written bytes : %d", writtenSize);

    float percent = 100.0f * ((float) writtenSize) / ((float)binarySize);

    if(lastPercent + 5 < percent){
        //echo("Uploading: " + String(percent) + "%");
        lastPercent = percent;
    }
}


void io_siiam$update_end$1_0_0(uint8_t length, uint8_t* frame){
    ESP_LOGE(TAG, "final written bytes : %d", writtenSize);

    esp_err_t err;
    err = esp_ota_end(update_handle);
    if (err!= ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed!");
        task_fatal_error();
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
        task_fatal_error();
    }

    char *str = nextString(&frame);
    setVersion(str);
    saveConfig();
    ESP_LOGI(TAG, "Write version: %s", str);
    free(str);

    ESP_LOGI(TAG, "Prepare to restart system!");
    esp_restart();
}