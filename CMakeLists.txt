# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0115 NEW)
LIST(APPEND CMAKE_PROGRAM_PATH  "xtensa-lx106-elf/bin")
set(ENV{IDF_PATH} "/Users/chris/CLionProjects/siiam-device-rtos/ESP8266_RTOS_SDK")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)


project(siiam-device-rtos)
