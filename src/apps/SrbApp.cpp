#include "SrbApp.h"
#include "TinyRandom.h"

const uint8_t level1[] PROGMEM = {
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,

    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    
    0b10010000,
    0b10000000,
    0b10010000,
    0b10010000,
    0b10010000,
    0b10000001,
    0b10000000,
    0b11100000,
    
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b11100000,
    0b10000000,
    0b10000001,
    0b11110000,
    
    0b10000000,
    0b10000001,
    0b10000001,
    0b11110000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b00000000,
    0b10000000,
    0b10000000,
    0b10000000,
    
    0b10000000,
    0b10010000,
    0b10010000,
    0b10000100,
    0b10000101,
    0b10000101,
    0b00000100,
    0b10000000,
    
    0b10000100,
    0b10010100,
    0b10000001,
    0b10000001,
    0b10010000,
    0b10010000,
    0b10000000,
    0b10000000,
    
    0b10000000,
    0b10010000,
    0b10000000,
    0b10010100,
    0b10000000,
    0b10010000,
    0b10000001,
    0b10000001,
    
    0b10010000,
    0b10000000,
    0b10000100,
    0b10000100,
    0b10000001,
    0b10000100,
    0b10010100,
    0b10000100,
    
    0b10000001,
    0b11000000,
    0b11100000,
    0b10000000,
    0b11100000,
    0b11000000,
    0b10000000,
    0b10000000,
    
    0b11000000,
    0b11100000,
    0b00000000,
    0b11100000,
    0b11000000,
    0b10000000,
    0b10000000,
    0b11000000,
    
    0b10000000,
    0b10010000,
    0b10010000,
    0b10000000,
    0b10000001,
    0b10000001,
    0b10000000,
    0b11100000,
    
    0b11000000,
    0b11100000,
    0b11110000,
    0b11111000,
    0b11111100,
    0b10000000,
    0b10000000,
    0b10000000,

    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
};

constexpr uint8_t GEOMETRY_MASK = 0b11111100;
constexpr uint8_t META_MASK     = 0b00000011;
// Metadata: 00 none, 01 patrol, 10 flyer, 11 reserved.
// This full byte is reserved and must be checked before decoding a column.
constexpr uint8_t LEVEL_END = 0b11111111;

constexpr uint8_t PLAYER_COLUMN = 4;
constexpr uint8_t LEVEL_TITLE = 0x80; // ANIM_TICK is otherwise 0..15.
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
    ENEMIES = 12, // FOUR BYTES
};

constexpr uint8_t LEVEL_COUNT = 2;




// --------------------------------------------------
// Hjälpmetoder
// --------------------------------------------------

uint8_t clampU8(int16_t value, uint8_t minValue, uint8_t maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return (uint8_t)value;
}

// Level columns and their offsets fit in one byte; hash8 keeps 16-bit mixing.
uint8_t triangleWave(uint8_t x, uint8_t period) {
    if (period < 2) return 0;

    uint8_t p = x % period;
    uint8_t half = period / 2;

    if (p <= half)
        return p;

    return period - p;
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


// --------------------------------------------------
// 1. Mask + intervall
// Din ursprungliga idé.
// --------------------------------------------------

bool algMask(
    uint16_t x,
    uint16_t offset,
    uint8_t interval,
    uint8_t mask
) {
    if (x < offset || interval == 0)
        return false;

    uint16_t px = x - offset;

    if (px % interval != 0)
        return false;

    uint8_t index = (px / interval) & 7;

    return (mask >> index) & 1;
}


// --------------------------------------------------
// 2. Intervall som minskar med x
// Features kommer tätare längre fram.
// --------------------------------------------------

bool algProgressive(
    uint8_t x,
    uint8_t offset,
    uint8_t startInterval,
    uint8_t minInterval,
    uint8_t rate
) {
    if (x < offset || rate == 0)
        return false;

    uint8_t px = x - offset;

    uint8_t reduction = px / rate;

    uint8_t interval =
        reduction >= startInterval - minInterval
        ? minInterval
        : startInterval - reduction;

    return px % interval == 0;
}


// --------------------------------------------------
// 3. Svårighet i steg
// Exempel: var 64:e kolumn blir intervallet mindre.
// --------------------------------------------------

bool algSteps(
    uint16_t x,
    uint16_t offset,
    uint8_t startInterval,
    uint8_t minInterval,
    uint8_t sectionWidth
) {
    if (x < offset || sectionWidth == 0)
        return false;

    uint16_t px = x - offset;
    uint8_t section = px / sectionWidth;

    uint8_t interval =
        section >= startInterval - minInterval
        ? minInterval
        : startInterval - section;

    return px % interval == 0;
}


// --------------------------------------------------
// 4. Vågrörelse
// Tätare -> glesare -> tätare.
// --------------------------------------------------

bool algWave(
    uint8_t x,
    uint8_t offset,
    uint8_t baseInterval,
    uint8_t amplitude,
    uint8_t period
) {
    if (x < offset || period < 2)
        return false;

    uint8_t px = x - offset;

    uint8_t wave = triangleWave(px, period);
    uint8_t half = period / 2;

    uint16_t delta = 0;

    if (half > 0)
        delta = (static_cast<uint16_t>(wave) * amplitude) / half;

    uint8_t interval = baseInterval + delta;

    if (interval < 1)
        interval = 1;

    return px % interval == 0;
}


// --------------------------------------------------
// 5. Deterministisk slump
// threshold 0..255.
// Högre threshold = oftare.
// --------------------------------------------------

bool algRandom(
    uint16_t x,
    uint16_t offset,
    uint8_t seed,
    uint8_t threshold
) {
    if (x < offset)
        return false;

    return hash8(x - offset, seed) < threshold;
}


// --------------------------------------------------
// 6. Slump som blir vanligare längre fram
// --------------------------------------------------

bool algRandomProgressive(
    uint8_t x,
    uint8_t offset,
    uint8_t seed,
    uint8_t startThreshold,
    uint8_t maxThreshold,
    uint8_t rate
) {
    if (x < offset || rate == 0)
        return false;

    uint8_t px = x - offset;

    uint16_t threshold =
        startThreshold + px / rate;

    if (threshold > maxThreshold)
        threshold = maxThreshold;

    return hash8(px, seed) < threshold;
}


// --------------------------------------------------
// 7. Repeating run
// Exempel: 4 kolumner aktivt var 24:e kolumn.
// Bra för plattformar.
// --------------------------------------------------

bool algRun(
    uint8_t x,
    uint8_t offset,
    uint8_t period,
    uint8_t length
) {
    if (x < offset || period == 0)
        return false;

    const uint8_t px = x - offset;
    return (px % period) < length;
}


// --------------------------------------------------
// 8. Run som blir längre längre fram
// --------------------------------------------------

bool algGrowingRun(
    uint16_t x,
    uint16_t offset,
    uint8_t period,
    uint8_t startLength,
    uint8_t maxLength,
    uint16_t rate
) {
    if (x < offset || period == 0 || rate == 0)
        return false;

    uint16_t px = x - offset;

    uint8_t length =
        startLength + px / rate;

    if (length > maxLength)
        length = maxLength;

    return (px % period) < length;
}




uint8_t SrbApp::getLevelColumn(uint8_t x){
    switch(console.state[LEVEL]){
        
        default:
        case 1: {

            if (x >= sizeof(level1) + 16) return LEVEL_END;
            if (x >= sizeof(level1)) return B10111100;
            return pgm_read_byte(&level1[x]);

        }

        case 2: {

            if (x >= 255) return LEVEL_END;
            if (x >= 239) return B10111100;
            if (x == 238) return B10000000;

            uint8_t col = B10000000;

            // Hål blir tätare längre fram
            if (algProgressive(
                x, 
                20, 
                28, 
                8, 
                45  + console.state[SEED3] % 9
            ))
                col &= ~B10000000;

            // Plattformar i återkommande block
            if (algRun(
                x, 
                16, 
                20  + console.state[SEED2] % 7, 
                5
            ))
                col |= 1 << 4;

            // Hög plattform i vågor
            if (algWave(
                x, 
                39, 
                22, 
                8, 
                54 + console.state[SEED1] % 21
            ))
                col |= 1 << 2;

            // Lite deterministisk extra variation
            if (algRandomProgressive(
                x,
                100 + console.state[SEED2] % 5,
                console.state[SEED1], //console.state.levelSeed,
                5,
                45,
                5 + console.state[SEED3] % 3
            )) {
                col |= B11000000;
            }

            // Lite deterministisk extra variation
            if (algRandomProgressive(
                x,
                90 + console.state[SEED3] % 5,
                console.state[SEED2], //console.state.levelSeed,
                5,
                45,
                6 + console.state[SEED1] % 5
            )) {
                col |= B11100000;
            }

            if (algRandomProgressive(
                x,
                22 + console.state[SEED1] % 5,
                console.state[SEED3],
                5,
                38,
                3 + console.state[SEED2] % 3U
            )) {
                col |= B00000001;
            }

            return col;

        }

    }
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
            console.state[LEVEL] = console.state[LEVEL] >= LEVEL_COUNT
                ? 1 : console.state[LEVEL] + 1;
            startLevel();
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
    for(uint8_t i = 1; i <= 4; i++){
        console.setPixel(console.Width - i, 0, lives >= i);
    }
}

uint8_t SrbApp::getLives(){
    uint8_t lives = 0;
    for(uint8_t i = 1; i <= 4; i++){
        if (console.getPixel(console.Width - i, 0)) lives++;
    }
    return lives;
}

void SrbApp::begin() {
    console.state[LEVEL] = 1;
    startLevel();
}

void SrbApp::update() {
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
        if ((buttons & BTN_UP) && move(false)) return;
        if ((buttons & BTN_DOWN) && move(true)) return;
    }

    if (animTick == 0){
        moveEnemies();
    }

    drawPlayer((animTick % 8 )!= 0);
    drawEnemies((animTick % 3) != 0);

    console.state[ANIM_TICK] = (animTick + 1) % 16;

    if (getLives() == 0) startLevel(false);
}
