make flash -j8 &&
./mkspiffs -p 256 -c filesystem -s 0x1F0000 build/spiffs-image.bin &&
esptool.py --port /dev/cu.usbserial-1110 --chip esp8266 write_flash 0x200000 build/spiffs-image.bin

