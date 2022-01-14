
#include "../protocol.h"
#include "../time_lib.h"
#include "../core.h"

#include "commands.h"
#include "internal/NeoEsp8266RtosDmaMethod.h" // instead of NeoPixelBus.h

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <netdb.h>
#include <sys/socket.h>


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

typedef struct pixel{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t white;
} pixel_t;

typedef struct neo_pixel{
    pixel_t *sources;
    pixel_t *targets[2];
    uint8_t target_index = 0;
    float animate;
    size_t size;
} neo_pixel_t;


neo_pixel_t* neoPixel;

static void calcColor(pixel_t *source, pixel_t *target, pixel_t *current, float animate){
    for(size_t color = 0 ; color < 3 ; color++ ){
        auto colorSource = (float)((uint8_t*)source)[color];
        auto colorTarget = (float)((uint8_t*)target)[color];
        auto colorCurrent = (uint8_t)((colorSource - colorTarget) * animate + colorTarget);
        ((uint8_t*)current)[color] = colorCurrent;
    }
}

[[noreturn]] static void runNeoPixel(void *pvParameters){
    while(true){
        neoPixel->animate *= 0.9;
        pixel_t *sources = neoPixel->sources;
        pixel_t *targets = neoPixel->targets[neoPixel->target_index];

        uint8_t *pixels = method.getPixels();

        pixel_t current;

        for(size_t index = 0 ; index < neoPixel->size ; index++ ){
            auto *source = sources + index;
            auto *target = targets + index;
            calcColor(source, target, &current, neoPixel->animate);
            pixels[index * 3 + 0] = current.green;
            pixels[index * 3 + 1] = current.red;
            pixels[index * 3 + 2] = current.blue;
        }
        method.Update();
        vTaskDelay( 33 / portTICK_PERIOD_MS );
    }
}

void io_siiam$neopixel_create$1_0_0(uint8_t length, uint8_t* frame){
    uint16_t oid = nextUInt16(&frame);
    uint16_t pixel_size = nextUInt16(&frame);
    uint8_t pin = nextUInt8(&frame);
    /*
    if(neoPixel != NULL){
        delete[] neoPixel->sources;
        delete[] neoPixel->targets[0];
        delete[] neoPixel->targets[1];
        delete neoPixel;
    }*/

    neoPixel = new neo_pixel_t();
    neoPixel->sources = new pixel_t[pixel_size];
    neoPixel->targets[0] = new pixel_t[pixel_size];
    neoPixel->targets[1] = new pixel_t[pixel_size];
    neoPixel->animate = 0;
    neoPixel->target_index = 0;
    neoPixel->size = pixel_size;

    method.Initialize();
    xTaskCreate(&runNeoPixel, "run_neo_pixel" , 4096 , NULL , 2 , NULL ) ;
}

void io_siiam$neopixel_set$1_0_0(uint8_t length, uint8_t* frame){
    nextUInt16(&frame);
    auto size = nextUInt16(&frame);

    uint8_t hiddenTarget = 1 - neoPixel->target_index;
    pixel_t *targetPixels = neoPixel->targets[hiddenTarget];

    for(;size > 0;size--){
        uint16_t index = nextUInt16(&frame);
        uint8_t red = nextUInt8(&frame);
        uint8_t green = nextUInt8(&frame);
        uint8_t blue = nextUInt8(&frame);

        targetPixels[index] = {
            .red = red,
            .green = green,
            .blue = blue,
            .white = 0
        };
    }
}

void io_siiam$neopixel_show$1_0_0(uint8_t length, uint8_t* frame){
    auto oid = nextUInt16(&frame);

    float animate = neoPixel->animate;
    uint8_t hiddenTarget = 1 - neoPixel->target_index;
    pixel *targetsNext = neoPixel->targets[hiddenTarget];
    pixel *targetsCurrent = neoPixel->targets[neoPixel->target_index];
    foreach(index, neoPixel->size ){
        calcColor(neoPixel->sources + index, targetsCurrent + index, neoPixel->sources + index, animate );
        targetsCurrent[index] = targetsNext[index];
    }
    neoPixel->target_index = hiddenTarget;
    neoPixel->animate = 1;
}