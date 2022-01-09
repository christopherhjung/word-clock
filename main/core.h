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

#include "network/wifi_connect.h"
#pragma once

#define foreach(i, size) for( uint32_t i = 0 ; i < size ; i++ )

#define Booted 0
#define Ready 1
#define Initialized 2

typedef struct{
    uint32_t index;
    void (*fun)(uint8_t, uint8_t*);
    const char *name;
    bool needInitialized;
} library_t;

extern uint8_t buffer[1024];
extern uint32_t bufferIndex;
extern bool initialized;

extern library_t libraries[256];
extern uint32_t libraryHead;

extern uint8_t debug_level;

extern uint8_t sendBuffer[128];

void setDescriptor(int);

void sendString(const char *str);

void startFrame(uint8_t** ptr, uint16_t command);

void endFrame( uint8_t* ptr, uint8_t* start);

int sendFrame(const uint8_t* arr, size_t len);

int32_t lookupCommandCode(const char* name);

void notifyState(uint8_t status);

void notifyLibrary(uint8_t command, const char*str);

void notifyLibraries();

void echo(const char* str);

void registerLibrary(const char *name, void (*fun)(uint8_t, uint8_t*), bool needInitialized);

void handleCoreInit(uint8_t length, uint8_t* frame);

int sendHex(const char* str, int len);

void initLibraries();

void initSiiam();

void runSiiam();