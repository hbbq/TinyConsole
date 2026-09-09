#pragma once

#include <stdint.h>

void tinyRandomSeed(uint16_t seed);
// Returns a value in [0, upperBound); a zero bound returns zero.
uint16_t tinyRandom(uint16_t upperBound);
