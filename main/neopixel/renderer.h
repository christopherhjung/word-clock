//
// Created by Christopher Jung on 18.04.24.
//

#pragma once

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
#include "display.h"

#include "lwip/apps/sntp.h"

void renderer_init();
void renderer_set_background_color(pixel_t color);
void renderer_set_foreground_color(pixel_t color);
void renderer_set_it_is_active(bool active);