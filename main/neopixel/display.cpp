

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <string.h>

#include "internal/NeoEsp8266RtosDmaMethod.h" // instead of NeoPixelBus.h

#include "display.h"
#include "../config.h"

Neo800KbpsMethod method(114, 3);

static const char *TAG = "neopixel";

static const uint8_t gamma[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,   2,   3,
    3,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,
    6,   6,   6,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,
    11,  11,  11,  12,  12,  13,  13,  13,  14,  14,  15,  15,  16,  16,  17,
    17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,  24,  25,
    25,  26,  27,  27,  28,  29,  29,  30,  31,  31,  32,  33,  34,  34,  35,
    36,  37,  38,  38,  39,  40,  41,  42,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
    64,  65,  66,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,  80,  81,
    82,  84,  85,  86,  88,  89,  90,  92,  93,  94,  96,  97,  99,  100, 102,
    103, 105, 106, 108, 109, 111, 112, 114, 115, 117, 119, 120, 122, 124, 125,
    127, 129, 130, 132, 134, 136, 137, 139, 141, 143, 145, 146, 148, 150, 152,
    154, 156, 158, 160, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180, 182,
    184, 186, 188, 191, 193, 195, 197, 199, 202, 204, 206, 209, 211, 213, 215,
    218, 220, 223, 225, 227, 230, 232, 235, 237, 240, 242, 245, 247, 250, 252,
255};

uint8_t meanderMap[] = {
    113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103,
    92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
    91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81,
    70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
    69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
    47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37,
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15,
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    3, 2, 1, 0
};

typedef struct neo_pixel{
    pixel_t *sources;
    pixel_t *targets[2];
    uint8_t target_index = 0;
    float animate;
    float current_brightness = 1.0;
    float target_brightness = 1.0;
    bool power = true;
    size_t size;
} neo_pixel_t;


neo_pixel_t* neoPixel;




pixel_t parse_pixel(const char* input){

    int red = 0, green = 0, blue = 0;
    int currentIndex = 0;

    // Find and extract red value
    while (input[currentIndex] != '(')
        currentIndex++;

    currentIndex++; // Move past '('

    while (input[currentIndex] != ',') {
        red = red * 10 + (input[currentIndex] - '0');
        currentIndex++;
    }

    currentIndex++; // Move past ','

    // Find and extract green value
    while (input[currentIndex] != ',') {
        green = green * 10 + (input[currentIndex] - '0');
        currentIndex++;
    }

    currentIndex++; // Move past ','

    // Find and extract blue value
    while (input[currentIndex] != ')') {
        blue = blue * 10 + (input[currentIndex] - '0');
        currentIndex++;
    }

    pixel_t pixel;
    pixel.red = red;
    pixel.green = green;
    pixel.blue = blue;

    return pixel;
}

pixel_t hsv_to_rgb(float h, float s, float v) {
    int i;
    float f, p, q, t;
    uint8_t r, g, b;
    if (s == 0) {
        // Achromatic (grey)
        r = g = b = (int)(v * 255);
    }else{
        h /= 60; // sector 0 to 5
        i = (int)h;
        f = h - i; // factorial part of h
        p = v * (1 - s);
        q = v * (1 - s * f);
        t = v * (1 - s * (1 - f));
        switch(i) {
            case 0:
                r = (uint8_t)(v * 255);
                g = (uint8_t)(t * 255);
                b = (uint8_t)(p * 255);
                break;
            case 1:
                r = (uint8_t)(q * 255);
                g = (uint8_t)(v * 255);
                b = (uint8_t)(p * 255);
                break;
            case 2:
                r = (uint8_t)(p * 255);
                g = (uint8_t)(v * 255);
                b = (uint8_t)(t * 255);
                break;
            case 3:
                r = (uint8_t)(p * 255);
                g = (uint8_t)(q * 255);
                b = (uint8_t)(v * 255);
                break;
            case 4:
                r = (uint8_t)(t * 255);
                g = (uint8_t)(p * 255);
                b = (uint8_t)(v * 255);
                break;
            default: // case 5:
                r = (uint8_t)(v * 255);
                g = (uint8_t)(p * 255);
                b = (uint8_t)(q * 255);
                break;
        }
    }

    return pixel_t {
        .red = r,
        .green = g,
        .blue = b,
        .white = 0
    };
}

static void interp_pixel(pixel_t *source, pixel_t *target, pixel_t *current, float animate, float brightness){
    for(size_t color = 0 ; color < 3 ; color++ ){
        auto colorSource = (float)((uint8_t*)source)[color];
        auto colorTarget = (float)((uint8_t*)target)[color];
        auto colorCurrent = (uint8_t)(((colorSource - colorTarget) * animate + colorTarget) * brightness);
        ((uint8_t*)current)[color] = colorCurrent;
    }
}

[[noreturn]] static void display_task(void *pvParameters){
    while(true){
        neoPixel->animate *= 0.9;
        float powered_brightness = neoPixel->target_brightness;
        if(!neoPixel->power){
            powered_brightness = 0.0;
        }

        neoPixel->current_brightness =
                neoPixel->current_brightness * 0.9 +
                powered_brightness * 0.1;

        pixel_t *sources = neoPixel->sources;
        pixel_t *targets = neoPixel->targets[neoPixel->target_index];

        uint8_t *pixels = method.getPixels();
        for(size_t index = 0 ; index < neoPixel->size ; index++ ){
            auto *source = sources + index;
            auto *target = targets + index;
            pixel_t current;
            interp_pixel(source, target, &current, neoPixel->animate, neoPixel->current_brightness);
            pixels[index * 3 + 0] = current.green;
            pixels[index * 3 + 1] = current.red;
            pixels[index * 3 + 2] = current.blue;
        }
        method.Update();
        vTaskDelay( 33 / portTICK_PERIOD_MS );
    }
}

void display_init(uint16_t pixel_size){
    neoPixel = new neo_pixel_t();
    neoPixel->sources = new pixel_t[pixel_size];
    neoPixel->targets[0] = new pixel_t[pixel_size];
    neoPixel->targets[1] = new pixel_t[pixel_size];
    neoPixel->animate = 0;
    neoPixel->target_index = 0;
    neoPixel->size = pixel_size;
    neoPixel->power = config_get_power_status();

    method.Initialize();
    xTaskCreate(&display_task, "display_task" , 4096 , NULL , 2 , NULL ) ;
}

void display_update(){
    pixel *target_current = neoPixel->targets[neoPixel->target_index];
    for(int index = 0 ; index < neoPixel->size; index++ ){
        interp_pixel(
            neoPixel->sources + index,
            target_current + index,
            neoPixel->sources + index,
            neoPixel->animate,
            1.0f
        );
    }

    neoPixel->target_index = 1 - neoPixel->target_index;
    neoPixel->animate = 1;
}

void display_set_brightness(float brightness){
    neoPixel->target_brightness = brightness;
}

void display_set_power(bool status){
    neoPixel->power = status;
    config_set_power_status(status);
    config_save();
}

void display_show(pixel_t* pixels){
    uint8_t hiddenTarget = 1 - neoPixel->target_index;
    pixel_t *targetPixels = neoPixel->targets[hiddenTarget];

    for(int idx = neoPixel->size;idx >= 0;idx--){
        targetPixels[meanderMap[idx]] = pixels[idx];
    }

    display_update();
}