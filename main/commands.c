

#include <sys/time.h>
#include <stdio.h>
#include <math.h>

#include "protocol.h"
#include "core.h"
#include "commands.h"

#include "driver/gpio.h"


#define GPIO_OUTPUT_IO_0    15
#define GPIO_OUTPUT_IO_1    16
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0     4
#define GPIO_INPUT_IO_1     5
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))

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
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    /*

    pinMode(pinNumber, mode);*/
}

void io_siiam$io_digitalWrite$1_0_0(uint8_t length, uint8_t* frame){
    int pinNumber = nextUInt8(&frame);
    int state = nextUInt8(&frame);
    gpio_set_level(pinNumber, state);
}

void io_siiam$io_digitalRead$1_0_0(uint8_t length, uint8_t* frame){
    static int32_t command = -2;

    if(command == -2){
        command = lookupCommandCode("io.siiam:io.digitalRead:1.0.0");
    }

    if(command >= 0){
        uint8_t pinNumber = nextUInt8(&frame);
        uint32_t readIndex = nextUInt32(&frame);
        int result = gpio_get_level(pinNumber);

        uint8_t *begin = sendBuffer;
        uint8_t *ptr = sendBuffer;
        startFrame(&ptr, command);
        appendUInt32(&ptr, readIndex);
        appendUInt8(&ptr, result);
        endFrame(&ptr, &begin);
        sendFrame(sendBuffer, sendBuffer[0]);
    }
}