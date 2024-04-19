

#include <sys/time.h>
#include <stdio.h>
#include <math.h>

#include "io.h"

#include "driver/gpio.h"
#include "driver/adc.h"

void pin_mode(uint8_t pin, uint8_t mode){
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = mode == 0 ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = 1ULL << pin;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void digital_write(uint8_t pin, uint32_t level){
    gpio_set_level((gpio_num_t)pin, level);
}

uint32_t digital_read(uint8_t pin){
    return gpio_get_level((gpio_num_t)pin);
}

uint16_t analog_read(){
    uint16_t adc_data = 0;
    adc_read(&adc_data);
    return adc_data;
}

