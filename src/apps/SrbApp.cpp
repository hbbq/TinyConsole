#include "SrbApp.h"
#include "TinyRandom.h"
#include <avr/eeprom.h>

namespace {
uint8_t EEMEM savedProgress[5];
}

const uint8_t level1Palette[] PROGMEM = {
    0x80, 0x90, 0x81, 0xE0, 0xF0, 0x00, 0x84,
    0x85, 0x04, 0x94, 0xC0, 0xF8, 0xFC
};

// Two palette indices per byte: even column in the low nibble.
const uint8_t level1[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x21, 0x30,
    0x00, 0x00, 0x03, 0x42, 0x20, 0x42, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,
    0x10, 0x61, 0x77, 0x08, 0x96, 0x22, 0x11, 0x00, 0x10, 0x90, 0x10, 0x22,
    0x01, 0x66, 0x62, 0x69, 0xA2, 0x03, 0xA3, 0x00, 0x3A, 0x35, 0x0A, 0xA0,
    0x10, 0x01, 0x22, 0x30, 0x3A, 0xB4, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00
};

constexpr uint8_t LEVEL1_COLUMNS = 120;

// Four two-bit platform rows per byte, from row 4 through row 7.
const uint8_t level8Rows[] PROGMEM = {
    0xaf, 0x9b, 0x27, 0xdb, 0x78, 0xdc, 0xc8
};

constexpr uint8_t GEOMETRY_MASK = 0b11111100;
constexpr uint8_t META_MASK     = 0b00000011;
// Metadata: 00 none, 01 patrol, 10 flyer, 11 reserved.
// This full byte is reserved and must be checked before decoding a column.
constexpr uint8_t LEVEL_END = 0b11111111;

constexpr uint8_t PLAYER_COLUMN = 4;
constexpr uint8_t LEVEL_TITLE = 0x80; // ANIM_TICK is otherwise 0..15.
constexpr uint8_t RESUME_MENU = 0x81;
constexpr uint8_t ENEMY_COUNT = 4;
constexpr uint8_t ENEMY_ACTIVE = 0x80;
constexpr uint8_t ENEMY_X_MASK = 0x78;
constexpr uint8_t ENEMY_X_STEP = 1 << 3;

constexpr uint8_t fixedPointShift = 4;
constexpr int8_t jumpVelocity = -16;
constexpr int8_t stompBounceVelocity = -12;
constexpr int8_t gravity = 2;
constexpr int8_t maximumFallVelocity = 8;

enum stateSlot {
    ANIM_TICK = 0,
    XPOS = 1,
    YPOS = 2,
    VELOCITY = 3,
    SEED1 = 4,
    SEED2 = 5,
    SEED3 = 6,
    LEVEL = 7,
    ENEMY_FLAGS = 8, // Low bits: right/up; high bits: flying. Clear = left/down, ground.
    LIVES = 9,
    ENEMIES = 12, // FOUR BYTES
};

constexpr uint8_t LEVEL_COUNT = 16;

static uint8_t savedChecksum(const uint8_t* data) {
    return data[0] ^ data[1] ^ data[2] ^ data[3];
}

bool SrbApp::hasSavedProgress(TinyConsoleGameApi& console) {
    uint8_t level = eeprom_read_byte(savedProgress);
    uint8_t check = level;
    for (uint8_t i = 0; i < 3; i++) {
        console.state[SEED1 + i] = eeprom_read_byte(savedProgress + i + 1);
        check ^= console.state[SEED1 + i];
    }
    console.state[LEVEL] = level;
    if (static_cast<uint8_t>(level - 2) >= LEVEL_COUNT - 1
        || check != eeprom_read_byte(savedProgress + 4)) return false;
    console.state[ANIM_TICK] = RESUME_MENU;
    return true;
}

void SrbApp::saveProgress() {
    // Level is the commit byte: invalidate it, write seeds/checksum, then commit.
    eeprom_write_byte(savedProgress, 0);
    eeprom_write_block(&console.state[SEED1], savedProgress + 1, 3);
    eeprom_write_byte(savedProgress + 4, savedChecksum(&console.state[SEED1]));
    eeprom_write_byte(savedProgress, console.state[LEVEL]);
}

void SrbApp::clearSavedProgress() {
    eeprom_write_byte(savedProgress, 0);
}




// --------------------------------------------------
// Hjälpmetoder
// --------------------------------------------------

uint8_t clampU8(int16_t value, uint8_t minValue, uint8_t maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return (uint8_t)value;
}

// Enkel deterministisk "slump" från x.
// Samma x + seed ger alltid samma resultat.
uint8_t hash8(uint16_t x, uint8_t seed) {
    x += seed * 31u;
    x ^= x >> 7;
    x *= 13u;
    x ^= x >> 5;
    return (uint8_t)x;
}

static uint8_t proceduralColumn(uint8_t x, uint8_t level,
                                uint8_t seed1, uint8_t seed2, uint8_t seed3) {
    if (x == 255) return LEVEL_END;
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
        uint8_t p = x & 7;
        uint8_t h = 5 + (hash8(x >> 3, seed1) & 1);
        uint8_t length = 7 - ((hash8(x >> 3, seed2) & 7) < density);
        col = p < length ? (1 << h) : 0;
    } else if (!(level & 1)) {
        if ((a & 7) < density && pos >= 4
            && pos < 5 + ((a >> 7) || intensity > 8)) col = 0;
        if ((b & 7) < density && pos >= 9 && pos <= 10)
            col = (b & 0x80) || intensity > 9 ? 0xe0 : 0xc0;
        if (pos <= 1) col |= 0x10;
    } else {
        if ((a & 7) < density) {
            uint8_t pattern = b & 3;
            if (intensity > 8 && pattern == 2) pattern = 3;
            if ((pattern == 0 || pattern == 3) && pos >= 4 && pos <= 5) col = 0;
            if ((pattern == 1 || pattern == 3) && pos >= 9 && pos <= 10) col = 0xc0;
            if (pattern == 2 && pos >= 4 && pos <= 6) col |= 0x10;
        }
        if (pos <= 1) col |= 0x10;
    }
    if (col && (hash8(x, seed3) & 63) < 2 + tier + (x >> 6)) col |= 1;
    return col;
}

uint8_t SrbApp::getLevelColumn(uint8_t x) {
    if (console.state[LEVEL] == 1) {
        if (x >= LEVEL1_COLUMNS + 16) return LEVEL_END;
        if (x >= LEVEL1_COLUMNS) return B10111100;
        uint8_t packed = pgm_read_byte(&level1[x >> 1]);
        uint8_t index = x & 1 ? packed >> 4 : packed & 0x0f;
        return pgm_read_byte(&level1Palette[index]);
    }
    if (console.state[LEVEL] == 8) {
        if (x == 255) return LEVEL_END;
        if (x >= 239) return 0xbc;
        if (x < 18) return 0xfc << (x / 3);

        uint8_t offset = x - 18;
        uint8_t cell = offset >> 3;
        if ((offset & 7) >= (x < 82 ? 7 : 6)) return 0;
        uint8_t packed = pgm_read_byte(&level8Rows[cell >> 2]);
        uint8_t column = 1 << (4 + ((packed >> ((cell & 3) * 2)) & 3));
        // One early cell offers a second, higher route.
        if (cell == 3) column |= column >> 2;
        return column;
    }
    return proceduralColumn(x, console.state[LEVEL], console.state[SEED1],
                            console.state[SEED2], console.state[SEED3]);
}

void SrbApp::startLevel(bool newLevel){
    // Retries reuse the layout; new games and progression (including wrap) reseed.
    if (newLevel) {
        console.state[SEED1] = tinyRandom(255);
        console.state[SEED2] = tinyRandom(255);
        console.state[SEED3] = tinyRandom(255);
    }
    console.showNumber(console.state[LEVEL]);
    console.state[ANIM_TICK] = LEVEL_TITLE;
    console.consumeButtons();
    console.tickDue(0);
}

void SrbApp::enterLevel(){
    console.state[ANIM_TICK] = 0;
    console.state[XPOS] = PLAYER_COLUMN;
    console.state[YPOS] = 0;
    console.state[VELOCITY] = 0;
    console.state[ENEMY_FLAGS] = 0;
    
    for(uint8_t x = 0; x < console.Width; x++){
        uint8_t column = getLevelColumn(x);
        if (column == LEVEL_END) break;
        console.insertColumn(x, column & GEOMETRY_MASK);
    }
    for(uint8_t index = 0; index < ENEMY_COUNT; index++){
        console.state[ENEMIES + index] = 0;
    }

    drawLives(3);

    drawPlayer(true);
}

bool SrbApp::move(bool right){
    uint8_t ypos = console.state[YPOS] >> fixedPointShift;
    uint8_t xpos = console.state[XPOS];
    uint8_t lives = getLives();

    if (right){
        if (console.getPixel(PLAYER_COLUMN + 1, ypos)) return false;
        uint8_t column = getLevelColumn(xpos - PLAYER_COLUMN + console.Width);
        if (column == LEVEL_END){
            if (++console.state[LEVEL] > LEVEL_COUNT) console.state[LEVEL] = 1;
            startLevel();
            if (console.state[LEVEL] > 1) saveProgress();
            else clearSavedProgress();
            return true;
        }
        console.shiftLeft();
        console.insertColumn(console.Width - 1, column & GEOMETRY_MASK);
        
        for(uint8_t* enemy = &console.state[ENEMIES];
            enemy != &console.state[ENEMIES + ENEMY_COUNT]; ++enemy){
            uint8_t data = *enemy;
            if (data & ENEMY_ACTIVE){
                // Move the packed X field, or deactivate at the screen edge.
                *enemy = (data & ENEMY_X_MASK)
                    ? data - ENEMY_X_STEP : data & ~ENEMY_ACTIVE;
            }
        }

        uint8_t meta = column & META_MASK;
        if (meta == 1 || meta == 2) {
            for(uint8_t index = 0; index < ENEMY_COUNT; index++){
                uint8_t data = console.state[ENEMIES + index];
                if (!(data & ENEMY_ACTIVE)){
                    console.state[ENEMIES + index] = ENEMY_ACTIVE | (15 << 3)
                        | (meta == 2 ? 3 : 1);
                    uint8_t direction = 1 << index;
                    uint8_t flying = direction << ENEMY_COUNT;
                    console.state[ENEMY_FLAGS] &= ~(direction | flying);
                    if (meta == 2) console.state[ENEMY_FLAGS] |= flying;
                    break;
                }
            }
        }
        xpos++;
    } else if (xpos > PLAYER_COLUMN){
        if (console.getPixel(PLAYER_COLUMN - 1, ypos)) return false;
        uint8_t column = getLevelColumn(xpos - PLAYER_COLUMN - 1);
        if (column == LEVEL_END) return false;
        console.shiftRight();
        console.insertColumn(0, column & GEOMETRY_MASK);   
        
        for(uint8_t* enemy = &console.state[ENEMIES];
            enemy != &console.state[ENEMIES + ENEMY_COUNT]; ++enemy){
            uint8_t data = *enemy;
            if (data & ENEMY_ACTIVE){
                *enemy = (data & ENEMY_X_MASK) == ENEMY_X_MASK
                    ? data & ~ENEMY_ACTIVE : data + ENEMY_X_STEP;
            }
        }
        xpos--;
    }
    
    console.state[XPOS] = xpos;
    drawLives(lives);
    for(uint8_t index = 0; index < ENEMY_COUNT; index++){
        checkEnemyCollision(index, true);
    }
    return false;
}

bool SrbApp::enemyBlocked(uint8_t x, uint8_t y) const {
    // Lives share row zero with the world, but are never enemy terrain.
    if (!(y == 0 && x >= console.Width - 4) && console.getPixel(x, y)) return true;
    // Falling below the screen must not wrap onto an enemy in row zero.
    if (y >= console.Height) return false;
    uint8_t position = ENEMY_ACTIVE | (x << 3) | y;
    // Use state, since enemy pixels are erased during physics and may blink.
    for (uint8_t index = 0; index < ENEMY_COUNT; index++) {
        if (console.state[ENEMIES + index] == position) return true;
    }
    return false;
}

void SrbApp::checkEnemyCollision(uint8_t index, bool hurtPlayer){
    uint8_t data = console.state[ENEMIES + index];
    if (!(data & ENEMY_ACTIVE)) return;
    if (((data & ENEMY_X_MASK) >> 3) != PLAYER_COLUMN) return;
    if ((data & B111) != (console.state[YPOS] >> fixedPointShift)) return;

    console.state[ENEMIES + index] = data & ~ENEMY_ACTIVE;
    if (hurtPlayer){
        uint8_t lives = getLives();
        if (lives > 0) drawLives(lives - 1);
    } else {
        // A stomp kicks the player upward, with less height than a jump.
        console.state[VELOCITY] = static_cast<uint8_t>(stompBounceVelocity);
    }
}

void SrbApp::drawPlayer(bool draw){
    uint8_t ypos = console.state[YPOS] >> fixedPointShift;

    console.setPixel(PLAYER_COLUMN, ypos, draw);
}

void SrbApp::doGravity(){
    int16_t ypos = static_cast<int16_t>(console.state[YPOS]);

    int8_t velocity = static_cast<int8_t>(console.state[VELOCITY]);

    velocity += gravity;
    if (velocity > maximumFallVelocity) velocity = maximumFallVelocity;

    if (velocity > 0 && console.getPixel(PLAYER_COLUMN, (ypos >> fixedPointShift) + 1)){
        velocity = 0;
        ypos = (ypos >> fixedPointShift) << fixedPointShift;
    }

    if (velocity < 0 && console.getPixel(PLAYER_COLUMN, ((ypos + velocity) >> fixedPointShift))){
        velocity = 0;
        ypos = (ypos >> fixedPointShift) << fixedPointShift;
    }

    console.state[VELOCITY] = static_cast<uint8_t>(velocity);
    ypos += velocity;

    if (ypos < 0) ypos = 0;

    if ((ypos >> fixedPointShift) >= 8) {
        uint8_t lives = getLives();
        lives--;
        drawLives(lives);
        console.setPixel(PLAYER_COLUMN, 7);
        ypos = 0;
    }
        
    console.state[YPOS] = static_cast<uint8_t>(ypos);

    // Resolve stomps before enemies get their own falling step.
    if (velocity > 0){
        for(uint8_t index = 0; index < ENEMY_COUNT; index++){
            checkEnemyCollision(index, false);
        }
    }
    
    for(uint8_t index = 0; index < ENEMY_COUNT; index++){
        uint8_t data = console.state[ENEMIES + index];
        bool active = (data >> 7) & B1;
        if (active && !(console.state[ENEMY_FLAGS] & (1 << (index + ENEMY_COUNT)))){
            uint8_t x = (data >> 3) & B1111;
            uint8_t y = (data >> 0) & B111;
            if (!enemyBlocked(x, y+1)) {
                y++;
                if (y >= 8) active = false;
                console.state[ENEMIES + index] =
                    (active ? 1 : 0) << 7 |
                    (x & B1111) << 3 |
                    (y & B111);
                checkEnemyCollision(index, true);
            }
        }
    }
}

void SrbApp::doJump(){
    uint8_t ypos = console.state[YPOS];
    if (console.getPixel(PLAYER_COLUMN, (ypos >> fixedPointShift) + 1)){
        console.state[VELOCITY] = static_cast<uint8_t>(jumpVelocity);
    }
}

void SrbApp::drawEnemies(bool draw){
    for(uint8_t index = 0; index < ENEMY_COUNT; index++){
        uint8_t data = console.state[ENEMIES + index];
        bool active = (data >> 7) & B1;
        if (active){
            uint8_t x = (data >> 3) & B1111;
            uint8_t y = (data >> 0) & B111;
            if (!(y == 0 && x >= console.Width - 4)) console.setPixel(x, y, draw);
        }
    }
}

void SrbApp::moveEnemies(){
    for(uint8_t index = 0; index < ENEMY_COUNT; index++){
        uint8_t data = console.state[ENEMIES + index];
        bool active = (data >> 7) & B1;
        if (active){
            uint8_t x = (data >> 3) & B1111;
            uint8_t y = (data >> 0) & B111;

            uint8_t direction = 1 << index;
            if (console.state[ENEMY_FLAGS] & (direction << ENEMY_COUNT)){
                uint8_t nextY = y + ((console.state[ENEMY_FLAGS] & direction) ? -1 : 1);
                if (nextY >= console.Height || enemyBlocked(x, nextY)){
                    console.state[ENEMY_FLAGS] ^= direction;
                    continue;
                }
                console.state[ENEMIES + index] = ENEMY_ACTIVE | (x << 3) | nextY;
                checkEnemyCollision(index, true);
            } else if (enemyBlocked(x, y+1)){
                uint8_t nextX = x + ((console.state[ENEMY_FLAGS] & direction) ? 1 : -1);
                // Check before packing X so either edge despawns instead of wrapping.
                if (nextX >= console.Width){
                    console.state[ENEMIES + index] = data & ~ENEMY_ACTIVE;
                    continue;
                }

                // Only terrain can provide a landing below an on-screen ledge.
                uint8_t landingY = y + 1;
                while (landingY < console.Height && !console.getPixel(nextX, landingY)) landingY++;
                if (enemyBlocked(nextX, y) || landingY == console.Height){
                    console.state[ENEMY_FLAGS] ^= direction;
                    continue;
                }

                console.state[ENEMIES + index] =
                    (1) << 7 |
                    (nextX & B1111) << 3 |
                    (y & B111);
                checkEnemyCollision(index, true);
            }
        }
    }
}

void SrbApp::drawLives(uint8_t lives){
    console.state[LIVES] = lives;
    for(uint8_t i = 1; i <= 4; i++){
        console.setPixel(console.Width - i, 0, lives >= i);
    }
}

uint8_t SrbApp::getLives(){
    return console.state[LIVES];
}

void SrbApp::begin() {
    if (console.state[ANIM_TICK] == RESUME_MENU) {
        // The saved level is the prompt: Up continues it, Down restarts at level 1.
        console.showNumber(console.state[LEVEL]);
        return;
    }
    console.state[LEVEL] = 1;
    startLevel();
}

void SrbApp::update() {
    if (console.state[ANIM_TICK] == RESUME_MENU) {
        uint8_t buttons = console.readButtons();
        if (buttons & BTN_UP) {
            startLevel(false);
        } else if (buttons & BTN_DOWN) {
            clearSavedProgress();
            console.state[LEVEL] = 1;
            startLevel();
        }
        return;
    }
    // Keep presenting the title without advancing gameplay or queuing presses.
    if (console.state[ANIM_TICK] == LEVEL_TITLE) {
        if (console.tickDue(1000)) enterLevel();
        return;
    }

    console.update();

    if (!console.tickDue(40)) return;

    uint8_t animTick = console.state[ANIM_TICK];

    drawPlayer(false);
    drawEnemies(false);

    uint8_t buttons = console.consumeButtons();

    if (buttons & BTN_ACTION) doJump();

    doGravity();

    if ((animTick % 4) == 0) {
        if (console.state[LEVEL] == 8) {
            if (move(true)) return;
        } else {
            if ((buttons & BTN_UP) && move(false)) return;
            if ((buttons & BTN_DOWN) && move(true)) return;
        }
    }

    if (animTick == 0){
        moveEnemies();
    }

    drawPlayer((animTick % 8 )!= 0);
    drawEnemies((animTick % 3) != 0);

    console.state[ANIM_TICK] = (animTick + 1) % 16;

    if (getLives() == 0) startLevel(false);
}
