
#include "core.h"

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

#include "wifi_connect.h"


static const char *CORE = "example";

uint8_t buffer[1024];
uint32_t bufferIndex = 0;
bool initialized = false;

uint8_t debug_level = 0;

uint8_t sendBuffer[128];

library_t libraries[256];
uint32_t libraryHead = 0;

static uint32_t currentId = 0;

#define BOOTED 0
#define READY 1
#define INITIALIZED 2

int stream = -1;

void setDescriptor(int s){
    stream = s;
}

int sendFrame(const uint8_t* arr, size_t len){
    if (write(stream, arr, len) < 0) {
        ESP_LOGE(CORE, "... socket send failed");
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        return -1;
    }

    return 0;
}

void sendString(const char* str){
    uint16_t size = strlen(str);
    sendFrame((const uint8_t*)&size, 2);
    sendFrame((const uint8_t*)str, size);
}

void datahex(const char* string, int len, uint8_t* data) {
    if((len % 2) != 0) // must be even
        return;

    size_t index = 0;
    while (index < len) {
        char c = string[index];
        uint8_t value;
        if(c >= '0' && c <= '9'){
            value = (c - '0');
        }else{
            c |= 1 << 5;
            if (c >= 'a' && c <= 'f'){
                value = (10 + (c - 'a'));
            }else{
                return;
            }
        }

        if( index % 2 == 0 ){
            data[index >> 1] = value << 4;
        }else{
            data[index >> 1] += value;
        }

        index++;
    }
}


int sendHex(const char* str, int len) {
    uint8_t array[len >> 1];
    datahex(str, len, array);
    return sendFrame(array, len >> 1);
}

void registerLibrary(const char* name, void (*fun)(uint8_t, uint8_t*), bool needInitialized){
    library_t library = {
        .index = libraryHead,
        .fun = fun,
        .name = name,
        .needInitialized = needInitialized
    };

    libraries[libraryHead] = library;
    libraryHead++;
}


void startSysFrame(uint8_t*& ptr, uint8_t command){
    ptr+=2;
    append(ptr, currentId++);
    append(ptr, (uint16_t)65535);
    append(ptr, command);
}

void startFrame(uint8_t*& ptr, uint16_t command){
    ptr+=2;
    append(ptr, currentId++);
    append(ptr, command);
}

void endFrame( uint8_t*& ptr, uint8_t* start) {
    *ptr++ = 0x7f;
    *(uint16_t*)start = ptr - start;
}

void append(uint8_t*& ptr, float value){
    append(ptr, *((uint32_t*)&value));
}

void append(uint8_t*& ptr, uint8_t value){
    *ptr++ = value;
}

void append(uint8_t*& ptr, uint16_t value){
    *ptr++ = value;
    *ptr++ = value >> 8;
}

void append(uint8_t*& ptr, uint32_t value){
    *ptr++ = value;
    *ptr++ = value >> 8;
    *ptr++ = value >> 16;
    *ptr++ = value >> 24;
}

void append(uint8_t*& ptr, uint16_t length, uint8_t* array){
    memcpy(ptr, array, length);
    ptr += length;
}

void notifyState(int status){
    uint8_t *ptr = sendBuffer;
    startSysFrame(ptr, 0);
    append(ptr, (uint8_t)status);
    endFrame(ptr, sendBuffer);
    sendFrame(sendBuffer, sendBuffer[0]);
}

void handleCoreInit(uint8_t length, uint8_t* frame){ //initialize
    initialized = true;
    notifyState(Initialized);
}

void echo(const char* str) {
    uint8_t *ptr = sendBuffer;
    startSysFrame(ptr, 2);
    append(ptr, str);
    endFrame(ptr, sendBuffer);
    sendFrame(sendBuffer, sendBuffer[0]);
}

void initLibraries(){
    registerLibrary("io.siiam:core.init:1.0.0", handleCoreInit, false);
    /*registerLibrary("io.siiam:core.token.assign:1.0.0", assignToken, false);
    registerLibrary("io.siiam:core.restart:1.0.0", restart, false);
    registerLibrary("io.siiam:core.setDebugLevel:1.0.0", setDebugLevel, false);
    registerLibrary("io.siiam:io.pinMode:1.0.0", io_siiam$io_pinMode$1_0_0);
    registerLibrary("io.siiam:io.digitalWrite:1.0.0", io_siiam$io_digitalWrite$1_0_0);
    registerLibrary("io.siiam:io.digitalRead:1.0.0", io_siiam$io_digitalRead$1_0_0);

    registerLibrary("io.siiam:stepper.create:1.0.0", io_siiam$stepper_create$1_0_0);
    registerLibrary("io.siiam:stepper.move:1.0.0", io_siiam$stepper_move$1_0_0);
    registerLibrary("io.siiam:motionSystem.create:1.0.0", io_siiam$motionSystem_create$1_0_0);
    registerLibrary("io.siiam:motionSystem.attach:1.0.0", io_siiam$motionSystem_attach$1_0_0);
    registerLibrary("io.siiam:motionSystem.move:1.0.0", io_siiam$motionSystem_move$1_0_0);

    registerLibrary("io.siiam:neopixel.create:1.0.0", io_siiam$neopixel_create$1_0_0);
    registerLibrary("io.siiam:neopixel.set:1.0.0", io_siiam$neopixel_set$1_0_0);
    registerLibrary("io.siiam:neopixel.show:1.0.0", io_siiam$neopixel_show$1_0_0);

    registerLibrary("io.siiam:update.begin:1.0.0", io_siiam$update_begin$1_0_0, false);
    registerLibrary("io.siiam:update.write:1.0.0", io_siiam$update_write$1_0_0, false);
    registerLibrary("io.siiam:update.end:1.0.0", io_siiam$update_end$1_0_0, false);*/
}

void notifyLibrary(uint8_t command, const char* str){
    uint8_t *ptr = sendBuffer;
    startSysFrame(ptr, 1);
    append(ptr, command);
    append(ptr, str);
    endFrame(ptr, sendBuffer);
    sendFrame(sendBuffer, sendBuffer[0]);
}

void notifyLibraries(){
    foreach(i, libraryHead){
        notifyLibrary(i, libraries[i].name);
    }
}