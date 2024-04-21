//
// Created by Christopher Jung on 21.04.24.
//

#pragma once

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

class OtaUpdate{
private:
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    uint32_t binarySize = 0;
    uint32_t writtenSize = 0;
    float lastPercent;
public:
    bool begin(uint32_t size);
    bool write(char* bytes, uint32_t size);
    bool finish();
};
