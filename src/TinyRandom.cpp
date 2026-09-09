#include "TinyRandom.h"

namespace {
uint16_t randomState;
}

void tinyRandomSeed(uint16_t seed) {
    // Xorshift must have a nonzero state, including when the launch counter wraps.
    randomState = seed ? seed : 1;
}

uint16_t tinyRandom(uint16_t upperBound) {
    if (upperBound == 0) return 0;

    // 16-bit xorshift with a period of 65535 for any nonzero seed.
    uint16_t value = randomState;
    value ^= value << 7;
    value ^= value >> 9;
    value ^= value << 8;
    randomState = value;
    return value % upperBound;
}
