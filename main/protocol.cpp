//
// Created by Christopher Jung on 23.10.21.
//

#include "protocol.h"
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

#include "protocol.h"


char* nextString(uint8_t** ptr){
    uint16_t versionLength = nextUInt16(ptr);
    char *arr = (char*)malloc(versionLength + 1);
    memcpy(arr, *ptr, versionLength);
    arr[versionLength] = 0;
    *ptr += versionLength;
    return arr;
}

uint32_t nextUInt32(uint8_t** ptr){
    uint32_t result = nextUInt8(ptr);
    result += nextUInt8(ptr) << 8;
    result += nextUInt8(ptr) << 16;
    result += nextUInt8(ptr) << 24;
    return result;
}

uint16_t nextUInt16(uint8_t** ptr){
    uint16_t result = nextUInt8(ptr);
    result += nextUInt8(ptr) << 8;
    return result;
}

uint8_t nextUInt8(uint8_t** ptr){
    return *((*ptr)++);
}

void appendFloat(uint8_t** ptr, float value){
    appendUInt32(ptr, *((uint32_t*)&value));
}

void appendUInt8(uint8_t** ptr, uint8_t value){
    **ptr = value;
    (*ptr)++;
}

void appendUInt16(uint8_t** ptr, uint16_t value){
    appendUInt8(ptr, value);
    appendUInt8(ptr, value >> 8);
}

void appendUInt32(uint8_t** ptr, uint32_t value){
    appendUInt8(ptr, value);
    appendUInt8(ptr, value >> 8);
    appendUInt8(ptr, value >> 16);
    appendUInt8(ptr, value >> 24);
}

void appendArray(uint8_t** ptr, uint8_t* array, uint16_t length){
    memcpy(*ptr, array, length);
    *ptr += length;
}


void appendString(uint8_t** ptr, const char* value){
    uint16_t size = strlen(value);
    appendUInt16(ptr, size);
    memcpy(*ptr, (uint8_t*)value, size);
    *ptr += size;
}