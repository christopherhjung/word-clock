//
// Created by Christopher Jung on 19.04.24.
//

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dimmer.h"
#include "../io/io.h"
#include "display.h"

const uint16_t min = 1000;
const uint16_t max = 850;

static void dimmer_task(void *arg)
{
    while (1) {
        uint16_t brightness = analog_read();
        float next_brightness = 1.0f;
        if(brightness > 10 ){
            float brightness_normalized = (float)(brightness - max) / (min - max);
            if(brightness_normalized > 1.0){
                brightness_normalized = 1.0;
            }else if(brightness_normalized < 0.0){
                brightness_normalized = 0.0;
            }
            next_brightness = 0.7f * (1 - brightness_normalized) + 0.3f;
        }
        display_set_brightness(next_brightness);
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}

void dimmer_init()
{
    xTaskCreate(dimmer_task, "dimmer_task", 1024, NULL, 10, NULL);
}
