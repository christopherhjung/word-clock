#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := siiam-device-rtos

CC = g++

EXTRA_COMPONENT_DIRS = NeoPixelBus_ESP8266_RTOS

COMPONENT_LDFLAGS = -lc -lrdimon -u _printf_float
COMPONENT_LDFRAGMENTS = -lc -lrdimon -u _printf_float

include $(IDF_PATH)/make/project.mk

