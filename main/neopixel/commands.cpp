
#include "../protocol.h"
#include "../time_lib.h"
#include "../core.h"

#include "commands.h"
#include "neopixel.h" // instead of NeoPixelBus.h

NeoEsp8266Dma800KbpsMethod method(114, 3);

void io_siiam$neopixel_create$1_0_0(uint8_t length, uint8_t* frame){

}

void io_siiam$neopixel_set$1_0_0(uint8_t length, uint8_t* frame){
    nextUInt16(&frame);
    auto size = nextUInt16(&frame);
    //uint8_t hiddenTarget = 1 - neoPixel->target_index;
    //pixel_t *targetPixels = neoPixel->targets[hiddenTarget];


    uint8_t *pixels = method.getPixels();

    for(;size > 0;size--){
        auto index = nextUInt16(&frame);
        auto red = nextUInt8(&frame);
        auto green = nextUInt8(&frame);
        auto blue = nextUInt8(&frame);

        pixels[index * 3] = green;
        pixels[index * 3 + 1] = blue;
        pixels[index * 3 + 2] = red;
    }
}

void io_siiam$neopixel_show$1_0_0(uint8_t length, uint8_t* frame){
    method.Update();
}

/*
typedef struct pixel{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t white;
} pixel_t;

typedef struct neo_pixel{
    Adafruit_NeoPixel *stripe;
    pixel_t *sources;
    pixel_t *currents;
    pixel_t *targets[2];
    uint8_t target_index = 0;
    float animate;
    size_t size;
    uint32_t lastUpdate;
} neo_pixel_t;

array_list_t<neo_pixel_t*> neoPixels;

void runNeoPixel(neo_pixel_t* neoPixel){
    uint32_t elapsed = elapsedTime(neoPixel->lastUpdate);
    if(elapsed > 33000){
        neoPixel->animate *= 0.9;
        auto *stripe = (Adafruit_NeoPixel*)neoPixel->stripe;
        pixel_t *sources = neoPixel->sources;
        pixel_t *targets = neoPixel->targets[neoPixel->target_index];
        pixel_t *currents = neoPixel->currents;

        for(size_t index = 0 ; index < neoPixel->size ; index++ ){
            pixel_t *source = sources + index;
            pixel_t *target = targets + index;
            pixel_t *current = currents + index;

            for(size_t color = 0 ; color < 3 ; color++ ){
                auto colorSource = (float)( (uint8_t*)source )[color];
                auto colorTarget = (float)( (uint8_t*)target )[color];
                auto colorCurrent = (uint8_t)((colorSource - colorTarget) * neoPixel->animate + colorTarget);
                *((uint8_t*)current + color) = Adafruit_NeoPixel::gamma8(colorCurrent);
            }

            stripe->setPixelColor(index, current->red, current->green, current->blue);
        }
        stripe->show();

        neoPixel->lastUpdate += elapsed;
    }
}

void runNeoPixels(){
    foreach( i, neoPixels.size ){
        runNeoPixel(neoPixels[i]);
    }
}

void io_siiam$neopixel_create$1_0_0(uint8_t length, uint8_t* frame){
    auto oid = next<uint16_t>(frame);
    auto pixel_size = next<uint16_t>(frame);
    auto pin = nextUInt8(frame);
    auto *stripe = new Adafruit_NeoPixel(pixel_size, pin, NEO_GRB + NEO_KHZ800);
    auto *neoPixel = new neo_pixel_t{
        .stripe = stripe,
        .sources = new pixel_t[pixel_size],
        .currents = new pixel_t[pixel_size],
        .targets = {
            new pixel_t[pixel_size],
            new pixel_t[pixel_size]
        },
        .size = pixel_size,
        .lastUpdate = 0
    };

    stripe->begin();
    foreach(i, pixel_size ){
        neoPixel->sources[i] = {
                .red = 0,
                .green = 0,
                .blue = 0
        };
        neoPixel->targets[0][i] = neoPixel->sources[i];
        neoPixel->targets[1][i] = neoPixel->sources[i];
        stripe->setPixelColor(i, 0, 0, 0);
    }
    stripe->show();
    refs[oid] = neoPixel;

    neoPixels.add(neoPixel);
    echo("createStripe(" + String(oid) + ", " + String(pixel_size) + ", " + String(pin) + ")" );
}

void io_siiam$neopixel_set$1_0_0(uint8_t length, uint8_t* frame){
    auto oid = next<uint16_t>(frame);
    auto size = next<uint16_t>(frame);
    auto *neoPixel = (neo_pixel_t*)refs[oid];
    uint8_t *current = frame;

    uint8_t hiddenTarget = 1 - neoPixel->target_index;
    pixel_t *targetPixels = neoPixel->targets[hiddenTarget];

    for(;size > 0;size--){
        auto index = next<uint16_t>(current);
        auto red = nextUInt8(current);
        auto green = nextUInt8(current);
        auto blue = nextUInt8(current);

        targetPixels[index] = {
            .red = red,
            .green = green,
            .blue = blue
        };

        echo("setPixel(" + String(oid) + ", " + String(index) + ", " + String(red) + ", " + String(green) + ", " + String(blue) + ")" );
    }
}

void io_siiam$neopixel_show$1_0_0(uint8_t length, uint8_t* frame){
    auto oid = next<uint16_t>(frame);
    auto *neoPixel = (neo_pixel_t*)refs[oid];

    uint8_t hiddenTarget = 1 - neoPixel->target_index;
    pixel *sourceTargets = neoPixel->targets[hiddenTarget];
    pixel *targetTargets = neoPixel->targets[neoPixel->target_index];
    foreach(index, neoPixel->size ){
        neoPixel->sources[index] = neoPixel->currents[index];
        targetTargets[index] = sourceTargets[index];
    }
    neoPixel->target_index = hiddenTarget;
    neoPixel->animate = 1;
    neoPixel->lastUpdate = micros();
    echo("showPixel()");
}*/