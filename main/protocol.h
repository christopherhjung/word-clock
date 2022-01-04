//
// Created by Christopher Jung on 23.10.21.
//

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"

char* nextString(uint8_t** ptr);

uint32_t nextUInt32(uint8_t** ptr);

uint16_t nextUInt16(uint8_t** ptr);

uint8_t nextUInt8(uint8_t** ptr);

char *nextString(uint8_t** ptr);