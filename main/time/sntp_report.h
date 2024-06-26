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

#include "lwip/apps/sntp.h"

static void initialize_sntp(void);

static void obtain_time(void);

static void sntp_example_task(void *arg);

void sntp_report_init();