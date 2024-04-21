//
// Created by Christopher Jung on 18.04.24.
//

#include "renderer.h"

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
#include "../io/io.h"
#include "lwip/apps/sntp.h"

#include "display.h"
#include "../config.h"


#define NONE 0
#define ES 1
#define IST 2
#define Ein 3
#define Eins 4
#define Zwei 5
#define Drei 6
#define Vier 7
#define Fuenf 8
#define Sechs 9
#define Sieben 10
#define Acht 11
#define Neun 12
#define Zehn 13
#define Elf 14
#define Zwoelf 15
#define MZwanzig 16
#define MFuenf 17
#define MZehn 18
#define Viertel 19
#define Vor 20
#define Nach 21
#define Halb 22
#define Uhr 23

#define EinsM 24  //Minuten Punkt 1
#define ZweiM 25  //Minuten Punkt 2
#define DreiM 26  //Minuten Punkt 3
#define VierM 27  //Minuten Punkt 4

#define SIGN(expression) ((expression) < 0 ? -1 : (expression) > 0 ? 1 : 0)
#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

static const char *TAG = "clock";

uint8_t wordStarts[] = { 0, 0, 3, 57, 57, 55, 67, 84, 73, 100, 60, 89, 80, 93, 77, 49, 15, 7, 11, 26, 39, 35, 44, 107, 110, 111, 112, 113 };
uint8_t wordLengths[] = { 0, 2, 3, 3, 4, 4, 4, 4, 4, 5, 6, 4, 4, 4, 3, 5, 7, 4, 4, 7, 3, 4, 4, 3, 1, 1, 1, 1 };

uint8_t currentBrightness = 0;
uint8_t targetBrightness = 0;
uint8_t adjustedBrightness = 50;

uint8_t upperBrightness = 100;
uint8_t lowerBrightness = 23;

uint8_t powerState = 1;

bool it_is_active = true;
uint8_t minutenMode = 1;


uint8_t find_words(uint8_t* targetWords, struct tm *time) {
    uint8_t* currentWord = targetWords;
    int hours = time->tm_hour;
    int minutes = time->tm_min;
    uint8_t minuteSegment = minutes / 5;

    switch (minuteSegment) {
        case 0: *currentWord++ = Uhr; break;
        case 1: case 5: case 7: case 11: *currentWord++ = MFuenf; break;
        case 2: case 10: *currentWord++ = MZehn; break;
        case 3: case 9: *currentWord++ = Viertel; break;
        case 4: case 8: *currentWord++ = MZwanzig; break;
    }

    switch (minuteSegment) {
        case 1: case 2: case 3: case 4: case 7: *currentWord++ = Nach; break;
        case 8: case 9: case 10: case 11: case 5: *currentWord++ = Vor; break;
    }

    switch (minuteSegment) {
        case 5: case 6: case 7: *currentWord++ = Halb; break;
    }

    int offset;
    if( hours == 1 && minuteSegment == 0 ){
        offset = 0;
    }else{
        offset = ((minuteSegment >= 5 ? 1 : 0) + 11 + hours ) % 12 + 1;
    }

    *currentWord++ = Ein + offset;

    for(int dot = 0 ; dot < minutes % 5 ; dot++){
        *currentWord++ = EinsM + dot;
    }

    if (it_is_active) {
        *currentWord++ = ES;
        *currentWord++ = IST;
    }

    return currentWord - targetWords;
}

pixel_t background_color = {
    .red = 0,
    .green = 0,
    .blue = 0,
    .white = 0
};

pixel_t foreground_color = {
    .red = 255,
    .green = 255,
    .blue = 255,
    .white = 0
};

struct Renderer{
    pixel_t pixels[114];

    void set_pixel(uint32_t idx, pixel_t pixel){
        pixels[idx] = pixel;
    }

    void clear(){
        for (uint8_t i = 0; i < 144; i++) {
            set_pixel(i, background_color);
        }
    }

    void show(){
        display_show(pixels);
    }
};

Renderer renderer;

void renderer_set_background_color(pixel_t color){
    background_color = color;
    config_set_background_color(color);
    config_save();
}

void renderer_set_foreground_color(pixel_t color){
    foreground_color = color;
    config_set_foreground_color(color);
    config_save();
}

void renderer_set_it_is_active(bool active){
    it_is_active = active;
    config_set_it_is_active(active);
    config_save();
}

void render_word(uint8_t targetWord) {
    uint8_t wordStart = wordStarts[targetWord];
    uint8_t wordLength = wordLengths[targetWord];

    for (uint8_t i = 0; i < wordLength; i++) {
        renderer.set_pixel(wordStart + i, foreground_color);
    }
}

void render_words(struct tm *time) {
    renderer.clear();

    uint8_t targetWords[16];
    uint8_t wordCount = find_words(targetWords, time);
    for (uint8_t i = 0; i < wordCount; i++) {
        render_word(targetWords[i]);
    }

    renderer.show();
}

#define AVERAGE_COUNT 10
uint32_t averageArray[AVERAGE_COUNT];
uint32_t averageIndex = 0;
uint32_t averageSum = 0;
uint32_t average = 0;
uint32_t nextBrightnessMeasurement = 0;
uint32_t measurementDelay = 50;

/*
void calcBrightness() {
    uint32_t currentTime = millis();
    if (nextBrightnessMeasurement < currentTime) {
        uint32_t newValue = analog_read();

        averageSum -= averageArray[averageIndex];
        averageSum += newValue;
        averageArray[averageIndex] = newValue;
        averageIndex = (averageIndex + 1) % AVERAGE_COUNT;
        average = averageSum / AVERAGE_COUNT;
        average = constrain(average, 920, 1023);
        //  targetBrightness = adjustedBrightness * map(average, 1023, 920, 100, 255) / 100; //IDK
        targetBrightness = map(average, 1023, 920, lowerBrightness, upperBrightness);
        if (powerState == 0) {
            targetBrightness = 0;
        }
        nextBrightnessMeasurement = currentTime + measurementDelay;
    }
}*/

uint8_t loading_circle[] = {
    24, 25, 26, 27, 28, 29,
    30, 41, 52, 63, 74, 85,
    96, 95, 94, 93, 92, 91,
    90, 79, 68, 57, 46, 35,
};

uint8_t loading_position = 0;



void render_loading(){
    renderer.clear();
    renderer.set_pixel(loading_circle[loading_position], foreground_color);

    loading_position = (loading_position + 1) % 24;
    renderer.show();
}

static void renderer_wait(void)
{
    time_t now;
    struct tm timeinfo;

    while (1) {
        time(&now);
        localtime_r(&now, &timeinfo);

        if (timeinfo.tm_year > 100){
            break;
        }

        render_loading();
        vTaskDelay(100 / portTICK_RATE_MS);
    }

    renderer.clear();
    renderer.show();
    vTaskDelay(1000 / portTICK_RATE_MS);
}

static void renderer_task(void *arg)
{

    renderer_wait();

    time_t now;
    struct tm timeinfo;

    while (1) {
        time(&now);
        localtime_r(&now, &timeinfo);
        ESP_LOGI(TAG, "Render Clock");
        render_words(&timeinfo);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void renderer_init()
{
    foreground_color = config_get_foreground_color();
    background_color = config_get_background_color();
    it_is_active = config_get_it_is_active();
    xTaskCreate(renderer_task, "renderer_task", 4096, NULL, 10, NULL);
}
