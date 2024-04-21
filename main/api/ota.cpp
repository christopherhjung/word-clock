//
// Created by Christopher Jung on 21.04.24.
//

#include "ota.h"

static const char *TAG="ota";

bool OtaUpdate::begin(uint32_t size){
    this->binarySize = size;
    ESP_LOGI(TAG, "Starting OTA of size %d", size);

    const esp_partition_t *to_boot;

    to_boot = esp_ota_get_boot_partition();
    if (!to_boot) {
        ESP_LOGI(TAG, "no OTA boot partition");
        to_boot = esp_ota_get_running_partition();
        if (!to_boot) {
            ESP_LOGE(TAG, "ERROR: Fail to get running partition");
            return false;
        }
    }

    update_partition = esp_ota_get_next_update_partition(NULL);
    if(update_partition == NULL){
        ESP_LOGE(TAG, "ERROR: Fail to get update partition");
        return false;
    }

    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);

    esp_err_t err = esp_ota_begin(update_partition, size, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
        return false;
    }
    ESP_LOGI(TAG, "esp_ota_begin succeeded");

    uint8_t ota_num = get_ota_partition_count();
    uint8_t update_ota_num = update_partition->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_0;

    ESP_LOGI(TAG, "Totoal OTA number %d update to %d part", ota_num, update_ota_num);
    return true;
}

bool OtaUpdate::write(char* bytes, uint32_t size){
    esp_err_t err = esp_ota_write( update_handle, (const void *)bytes, size);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
        return false;
    }

    writtenSize += size;
    float percent = 100.0f * ((float) writtenSize) / ((float)binarySize);

    if(lastPercent + 5 < percent){
        //echo("Uploading: " + String(percent) + "%");
        ESP_LOGI(TAG, "Uploading: %%%.2f", percent);
        lastPercent = percent;
    }

    return true;
}


bool OtaUpdate::finish(){
    ESP_LOGE(TAG, "final written bytes : %d", writtenSize);

    esp_err_t err;
    err = esp_ota_end(update_handle);
    if (err!= ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed!");
        return false;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
        return false;
    }

    return true;
}