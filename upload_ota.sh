#!/bin/bash

SERIAL=${1:-"/dev/cu.usbserial-2110"}

export PATH="$PATH:$(realpath xtensa-lx106-elf/bin)"
export IDF_PATH=$(realpath ESP8266_RTOS_SDK)

eval "$(pyenv init --path)"
pyenv global 3.10.1

make -j8 &&
curl -X POST --data-binary "@build/siiam-device-rtos.bin" http://word-clock/ota



