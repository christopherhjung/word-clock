#!/bin/bash

SERIAL=${1:-"/dev/cu.usbserial-1110"}

git submodule update --init
brew install wget

wget https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-macos.tar.gz
tar -xf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-macos.tar.gz

set PATH "$PATH:$(realpath xtensa-lx106-elf/bin)"
git submodule update --init --recursive
set IDF_PATH=$(realpath ESP8266_RTOS_SDK)


make -j8 &&
./mkspiffs -p 256 -c filesystem -s 0x1F0000 build/spiffs-image.bin &&
python ESP8266_RTOS_SDK/components/esptool_py/esptool/esptool.py --chip esp8266 --port $SERIAL --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB \
 0xd000 build/ota_data_initial.bin \
 0x0 build/bootloader/bootloader.bin \
 0x10000 build/siiam-device-rtos.bin \
 0x8000 build/partitions.bin \
 0x200000 build/spiffs-image.bin



