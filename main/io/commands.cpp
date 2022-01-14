

#include <sys/time.h>
#include <stdio.h>
#include <math.h>

#include "../protocol.h"
#include "../core.h"
#include "commands.h"

#include "driver/gpio.h"
#include "driver/adc.h"

void io_siiam$io_pinMode$1_0_0(uint8_t length, uint8_t* frame){
    int pinNumber = nextUInt8(&frame);
    int mode = nextUInt8(&frame);

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = mode == 0 ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = 1ULL << pinNumber;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void io_siiam$io_digitalWrite$1_0_0(uint8_t length, uint8_t* frame){
    int pinNumber = nextUInt8(&frame);
    int state = nextUInt8(&frame);
    gpio_set_level((gpio_num_t)pinNumber, state);
}

void io_siiam$io_digitalRead$1_0_0(uint8_t length, uint8_t* frame){
    static int32_t command = -2;

    if(command == -2){
        command = lookupCommandCode("io.siiam:io.digitalRead:1.0.0");
    }

    if(command >= 0){
        uint8_t pinNumber = nextUInt8(&frame);
        uint32_t readIndex = nextUInt32(&frame);
        int result = gpio_get_level((gpio_num_t)pinNumber);

        uint8_t *ptr = sendBuffer;
        startFrame(&ptr, command);
        appendUInt32(&ptr, readIndex);
        appendUInt8(&ptr, result);
        endFrame(ptr, sendBuffer);
        sendFrame(sendBuffer, sendBuffer[0]);
    }
}

void io_siiam$io_analogRead$1_0_0(uint8_t length, uint8_t* frame){
    static int32_t command = -2;

    if(command == -2){
        command = lookupCommandCode("io.siiam:io.analogRead:1.0.0");
    }

    if(command >= 0){
        uint8_t pinNumber = nextUInt8(&frame);
        uint32_t readIndex = nextUInt32(&frame);

        uint16_t adc_data = 0;
        adc_read(&adc_data);

        uint8_t *ptr = sendBuffer;
        startFrame(&ptr, command);
        appendUInt32(&ptr, readIndex);
        appendUInt16(&ptr, adc_data);
        endFrame(ptr, sendBuffer);
        sendFrame(sendBuffer, sendBuffer[0]);
    }
}

