

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

void io_siiam$update_begin$1_0_0(uint8_t length, uint8_t* frame);

void io_siiam$update_write$1_0_0(uint8_t length, uint8_t* frame);

void io_siiam$update_end$1_0_0(uint8_t length, uint8_t* frame);
