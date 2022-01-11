#!/bin/bash

git submodule update --init --recursive
brew cleanup
brew install wget coreutils pyenv

if [ ! -d "xtensa-lx106-elf" ]; then
  wget https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-macos.tar.gz
  tar -xf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-macos.tar.gz
  rm xtensa-lx106-elf-gcc*.tar.gz
fi

pyenv install 3.10.1 -s
eval "$(pyenv init --path)"
pyenv global 3.10.1

python -m pip install --user -r $(realpath ESP8266_RTOS_SDK/requirements.txt)