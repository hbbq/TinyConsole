#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>
#include <binary.h>

using byte = uint8_t;
unsigned long millis();
int analogRead(uint8_t pin);
void delay(unsigned long duration);
