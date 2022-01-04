
#include "core.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include <sys/socket.h>

#include "config.h"

#include "protocol.h"

#include "io/commands.h"
#include "updater/commands.h"
//#include "neopixel/commands.h"

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

void appendString(uint8_t** ptr, const char* value){
    uint16_t size = strlen(value);
    appendUInt16(ptr, size);
    memcpy(*ptr, (uint8_t*)value, size);
    *ptr += size;
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


void startSysFrame(uint8_t** ptr, uint8_t command){
    *ptr+=2;
    appendUInt32(ptr, currentId++);
    appendUInt16(ptr, (uint16_t)65535);
    appendUInt8(ptr, command);
}

void startFrame(uint8_t** ptr, uint16_t command){
    *ptr+=2;
    appendUInt32(ptr, currentId++);
    appendUInt16(ptr, command);
}

void endFrame( uint8_t** ptr, uint8_t** start) {
    appendUInt8(ptr, 0x7f);
    *(*(uint16_t**)start) = *ptr - *start;
}

void appendFloat(uint8_t** ptr, float value){
    appendUInt32(ptr, *((uint32_t*)&value));
}

inline void appendUInt8(uint8_t** ptr, uint8_t value){
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

void notifyState(uint8_t status){
    uint8_t *begin = sendBuffer;
    uint8_t *ptr = sendBuffer;
    startSysFrame(&ptr, 0);
    appendUInt8(&ptr, status);
    endFrame(&ptr, &begin);
    sendFrame(sendBuffer, sendBuffer[0]);
}

void handleCoreInit(uint8_t length, uint8_t* frame){ //initialize
    initialized = true;
    notifyState(Initialized);
}

void echo(const char* str) {
    uint8_t *begin = sendBuffer;
    uint8_t *ptr = sendBuffer;
    startSysFrame(&ptr, 2);
    appendString(&ptr, str);
    endFrame(&ptr, &begin);
    sendFrame(sendBuffer, sendBuffer[0]);
}

int32_t lookupCommandCode(const char *name){
    foreach(i, libraryHead){
        if(strcmp(libraries[i].name, name) == 0){
            return (int32_t)libraries[i].index;
        }
    }

    return -1;
}

void initLibraries(){
    registerLibrary("io.siiam:core.init:1.0.0", handleCoreInit, false);

    registerLibrary("io.siiam:io.pinMode:1.0.0", io_siiam$io_pinMode$1_0_0, true);
    registerLibrary("io.siiam:io.digitalWrite:1.0.0", io_siiam$io_digitalWrite$1_0_0, true);
    registerLibrary("io.siiam:io.digitalRead:1.0.0", io_siiam$io_digitalRead$1_0_0, true);
/*
    registerLibrary("io.siiam:neopixel.create:1.0.0", io_siiam$neopixel_create$1_0_0);
    registerLibrary("io.siiam:neopixel.set:1.0.0", io_siiam$neopixel_set$1_0_0);
    registerLibrary("io.siiam:neopixel.show:1.0.0", io_siiam$neopixel_show$1_0_0);*/

    /*registerLibrary("io.siiam:core.token.assign:1.0.0", assignToken, false);
    registerLibrary("io.siiam:core.restart:1.0.0", restart, false);
    registerLibrary("io.siiam:core.setDebugLevel:1.0.0", setDebugLevel, false);

    registerLibrary("io.siiam:stepper.create:1.0.0", io_siiam$stepper_create$1_0_0);
    registerLibrary("io.siiam:stepper.move:1.0.0", io_siiam$stepper_move$1_0_0);
    registerLibrary("io.siiam:motionSystem.create:1.0.0", io_siiam$motionSystem_create$1_0_0);
    registerLibrary("io.siiam:motionSystem.attach:1.0.0", io_siiam$motionSystem_attach$1_0_0);
    registerLibrary("io.siiam:motionSystem.move:1.0.0", io_siiam$motionSystem_move$1_0_0);*/


    registerLibrary("io.siiam:update.begin:1.0.0", io_siiam$update_begin$1_0_0, false);
    registerLibrary("io.siiam:update.write:1.0.0", io_siiam$update_write$1_0_0, false);
    registerLibrary("io.siiam:update.end:1.0.0", io_siiam$update_end$1_0_0, false);
}

void notifyLibrary(uint8_t command, const char* str){
    uint8_t *begin = sendBuffer;
    uint8_t *ptr = sendBuffer;
    startSysFrame(&ptr, 1);
    appendUInt8(&ptr, command);
    appendString(&ptr, str);
    endFrame(&ptr, &begin);
    sendFrame(sendBuffer, sendBuffer[0]);
}

void notifyLibraries(){
    foreach(i, libraryHead){
        notifyLibrary(i, libraries[i].name);
    }
}

void ackFrame(uint32_t id){
    uint8_t *begin = sendBuffer;
    uint8_t *ptr = sendBuffer;
    startSysFrame(&ptr, 3);
    appendUInt32(&ptr, id);
    endFrame(&ptr, &begin);
    sendFrame(sendBuffer, sendBuffer[0]);
}

void handle() {
    uint8_t *ptr = buffer;
    uint16_t size = nextUInt16(&ptr);
    uint32_t id = nextUInt32(&ptr);
    uint8_t command = nextUInt8(&ptr);
    library_t *lib = libraries + command;
    if(initialized || !lib->needInitialized){
        if(lib->fun != NULL){
            lib->fun(size - 8, ptr);
        }
    }
    ackFrame(id);
}

const char *api =   "eccb238996d80df0218bfff6bda602368592c7ec3567367a37bde1acd7d79962";
const char *token_default = "d7a2c4db4329895530e6b7fa1a93bbbd7254caa2f221d2cf692cbe743dc35af2";

char token[64];
char version[64];

void initSiiam(){
    mount();

    memset(version, 0, 64);
    memcpy(token, token_default, 64);

    readFile("/spiffs/version", version, 64);
    ESP_LOGI("core", "Read version: %s", version);

    readFile("/spiffs/token", token, 64);
    ESP_LOGI("core", "Read token: %s", token);

    if(sendHex(api, 64) != 0){
        return;
    }

    sendString(version);

    if(sendHex(token, 64) != 0){
        return;
    }

    notifyState(Booted);
    notifyLibraries();
    notifyState(Ready);

    static uint8_t sync[32];
    int counter = 0;
    for(;;){
        ssize_t received = recv(stream, &sync, 32 - counter, 0);
        for(int i = 0 ; i < received ; i++){
            if(sync[i] == 0x7f){
                counter++;
                if(counter == 32){
                    goto sync;
                }
            }else{
                counter = 0;
            }
        }
    }

    sync:;
}

void runSiiam() {
    ssize_t received = recv(stream, &buffer, 2, 0);
    if(received == 2){
        uint16_t frameSize = *(uint16_t*)buffer;
        for(;;){
            received += recv(stream, buffer + received, frameSize - received, 0);
            if(received >= frameSize){
                break;
            }
        }

        if (buffer[received - 1] == 0x7f) {
            handle();
        }
    }
}