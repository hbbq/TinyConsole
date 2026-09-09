// Experiment only: replacement for the old procedural helpers/level-2 body.
// Uses the existing hash8 (including its 16-bit wrap), no persistent state.
static uint8_t proceduralColumn(uint8_t x, uint8_t level,
                                uint8_t seed1, uint8_t seed2, uint8_t seed3) {
    if (x == 255) return 255;
    if (x >= 239) return 0xbc;
    if (x < 16) return x >= 6 && x <= 7 ? 0x90 : 0x80;
    if (x >= 232) return 0x80;

    uint8_t tier = (level - 1) / 5;
    if (tier > 3) tier = 3;
    uint8_t intensity = 5 + tier + (x >> 6);
    uint8_t density = intensity > 8 ? 8 : intensity;
    uint8_t cell = x >> 4;
    uint8_t pos = x & 15;
    uint8_t a = hash8(cell, seed1);
    uint8_t b = hash8(cell, seed2);
    uint8_t col = 0x80;

    if (level % 5 == 0) {
        // Six/seven-column landings, one/two-column gaps, <= one row rise.
        uint8_t p = x & 7;
        uint8_t h = 5 + (hash8(x >> 3, seed1) & 1);
        uint8_t length = 7 - ((hash8(x >> 3, seed2) & 7) < density);
        col = p < length ? (1 << h) : 0;
    } else if (!(level & 1)) {
        // Independent decisions, separated spatially to reserve jump clearance.
        if ((a & 7) < density && pos >= 4
            && pos < 5 + ((a >> 7) || intensity > 8)) col = 0;
        if ((b & 7) < density && pos >= 9 && pos <= 10)
            col = (b & 0x80) || intensity > 9 ? 0xe0 : 0xc0;
        if (pos <= 1) col |= 0x10;
    } else {
        if ((a & 7) < density) {
            // Four proven patterns: hole, low step, raised platform, hole + step.
            uint8_t pattern = b & 3;
            if (intensity > 8 && pattern == 2) pattern = 3;
            if ((pattern == 0 || pattern == 3) && pos >= 4 && pos <= 5) col = 0;
            if ((pattern == 1 || pattern == 3) && pos >= 9 && pos <= 10) col = 0xc0;
            if (pattern == 2 && pos >= 4 && pos <= 6) col |= 0x10;
        }
        if (pos <= 1) col |= 0x10;
    }
    // Initial overall enemy rate: 2..8 / 64 per eligible column.
    // Enemy mix/eligibility remains a separate Todo. Only patrols in prototype.
    if (col && (hash8(x, seed3) & 63) < 2 + tier + (x >> 6)) col |= 1;
    return col;
}
