#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := siiam-device-rtos

CC = g++

EXTRA_COMPONENT_DIRS = NeoPixelBus_ESP8266_RTOS

include $(IDF_PATH)/make/project.mk

