//
// Created by Christopher Jung on 23.10.21.
//

#include "protocol.h"

String nextString(uint8_t*& ptr){
    auto versionLength = next<uint16_t>(ptr);
    char arr[versionLength + 1];
    memcpy(arr, ptr, versionLength);
    arr[versionLength] = 0;
    ptr += versionLength;
    return {(char*)arr};
}