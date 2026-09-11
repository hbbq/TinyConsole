#include "GameIcons.h"

namespace {
// GameId order. Four columns from left to right; bit 0 is the top row.
// Bits 0..5 hold the six icon rows; bits 6..7 stay blank.
const uint8_t icons[][4] PROGMEM = {

#if ENABLE_RACER
    // ####
    // ...#.
    // #...
    // ....
    // ##..
    // ####
    { 0b110101, 0b110001, 0b100001, 0b100011 }, // Racer
#endif

#if ENABLE_BREAKOUT
    // ####
    // #.##
    // ....
    // ..#.
    // ....
    // .##.
    { 0b000011, 0b100001, 0b101011, 0b000011 }, // Breakout
#endif
    
#if ENABLE_SKYHOP
    // ..#.
    // ..#.
    // ....
    // #.#.
    // ..#.
    // ..#.
    { 0b001000, 0b000000, 0b111011, 0b000000 }, // SkyHop
#endif
    
#if ENABLE_SHIFT
    // .#..
    // .#..
    // ###.
    // .##.
    // .###
    // .###
    { 0b000100, 0b111111, 0b111100, 0b110000 }, // Shift
#endif

#if ENABLE_SRB
    // ....
    // ....
    // ..##
    // ....
    // .#..
    // ####
    { 0b100000, 0b110000, 0b100100, 0b100100 }  // Srb
#endif
};
static_assert(sizeof(icons) / sizeof(icons[0]) == gameCount,
              "Provide one 4x6 icon for every GameId");
}

uint8_t gameIconColumn(GameId id, uint8_t column) {
    return pgm_read_byte(&icons[static_cast<uint8_t>(id)][column]);
}
