#pragma once

void pin_mode(uint8_t pin);
void digital_write(uint8_t pin, uint32_t level);
uint32_t digital_read(uint8_t pin);
uint16_t analog_read();