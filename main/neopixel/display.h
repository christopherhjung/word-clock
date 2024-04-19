

#pragma once

typedef struct pixel{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t white;
} pixel_t;

pixel_t parse_pixel(const char* input);
void display_init(uint16_t pixel_size);
void display_show(pixel_t* pixels);
void display_set_brightness(float brightness);
void display_set_power(bool status);