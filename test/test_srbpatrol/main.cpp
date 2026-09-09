#include "apps/SrbApp.h"

// Run the real game and framebuffer on the AVR simulator; only time and ADC
// input are supplied by the test so each physics step is deterministic.
unsigned long now;
int buttonAdc = 1023;
unsigned long millis() { return now; }
int analogRead(uint8_t) { return buttonAdc; }
void delay(unsigned long duration) { now += duration; }
uint8_t checkLauncher();
volatile uint8_t launcherFailureLine;

static TinyConsoleGameApi console;
static SrbApp game(console);
constexpr uint8_t TICK = 0, X = 1, Y = 2, V = 3, LEVEL = 7, DIRECTIONS = 8, ENEMIES = 12;
volatile uint16_t failureLine;
volatile uint16_t checks;
volatile bool completed;

extern "C" __attribute__((noinline)) void testFinished() {
    asm volatile("nop");
}

#define CHECK(condition) do { \
    ++checks; \
    if (!(condition)) { failureLine = __LINE__; testFinished(); for (;;) {} } \
} while (0)

static uint8_t enemy(uint8_t x, uint8_t y) {
    return 0x80 | (x << 3) | y;
}

static void reset() {
    console.clearState();
    console.clearScreen();
    console.consumeButtons();
    console.state[X] = 4;
    console.state[Y] = 6 << 4;
    console.state[LEVEL] = 1;
    for (uint8_t x = 0; x < 16; x++) console.setPixel(x, 7);
    game.drawLives(3);
    buttonAdc = 1023;
    console.tickDue(0);
}

static void tick(uint16_t elapsed = 40) {
    now += elapsed;
    game.update();
}

static void patrol() {
    reset();
    console.state[ENEMIES] = enemy(2, 6);
    game.moveEnemies();
    CHECK(console.state[ENEMIES] == enemy(1, 6)); // Moves away from player.

    // All four direction bits are independent; turns consume a whole step.
    for (uint8_t index = 0; index < 4; index++) {
        reset();
        console.state[DIRECTIONS] = 0x0f & ~(1 << index);
        console.state[ENEMIES + index] = enemy(8, 6);
        console.setPixel(7, 6);
        game.moveEnemies();
        CHECK(console.state[ENEMIES + index] == enemy(8, 6));
        CHECK(console.state[DIRECTIONS] == 0x0f);
        game.moveEnemies();
        CHECK(console.state[ENEMIES + index] == enemy(9, 6));
        console.setPixel(10, 6);
        game.moveEnemies();
        CHECK(console.state[ENEMIES + index] == enemy(9, 6));
        CHECK(console.state[DIRECTIONS] == (0x0f & ~(1 << index)));
    }

    reset();
    console.state[ENEMIES] = enemy(8, 6);
    console.state[ENEMIES + 1] = enemy(7, 6);
    console.state[DIRECTIONS] = 2;
    game.moveEnemies(); // Neither enemy is drawn: collision must use state.
    CHECK(console.state[ENEMIES] == enemy(8, 6));
    CHECK(console.state[ENEMIES + 1] == enemy(7, 6));
    CHECK(console.state[DIRECTIONS] == 1);
    game.moveEnemies();
    CHECK(console.state[ENEMIES] == enemy(9, 6));
    CHECK(console.state[ENEMIES + 1] == enemy(6, 6));

    reset();
    console.state[ENEMIES] = enemy(0, 6);
    console.state[ENEMIES + 1] = enemy(15, 6);
    console.state[DIRECTIONS] = 2;
    game.moveEnemies();
    CHECK(!(console.state[ENEMIES] & 0x80));
    CHECK(!(console.state[ENEMIES + 1] & 0x80));
}

static void ledgesAndGravity() {
    reset();
    console.state[ENEMIES] = enemy(8, 3);
    console.setPixel(8, 4);
    game.moveEnemies();
    CHECK(console.state[ENEMIES] == enemy(7, 3));
    game.moveEnemies(); // Unsupported enemies wait for gravity.
    CHECK(console.state[ENEMIES] == enemy(7, 3));
    for (uint8_t y = 4; y <= 6; y++) {
        game.doGravity();
        CHECK(console.state[ENEMIES] == enemy(7, y));
    }
    game.doGravity();
    CHECK(console.state[ENEMIES] == enemy(7, 6));

    reset();
    console.state[ENEMIES] = enemy(8, 3);
    console.setPixel(8, 4);
    console.insertColumn(7, 0);
    game.moveEnemies();
    CHECK(console.state[ENEMIES] == enemy(8, 3));
    CHECK(console.state[DIRECTIONS] == 1);
    console.insertColumn(9, 1 << 2); // Terrain above is not a landing.
    game.moveEnemies();
    CHECK(console.state[ENEMIES] == enemy(8, 3));
    CHECK(console.state[DIRECTIONS] == 0);

    // An enemy below the ledge must not be mistaken for landing terrain.
    console.state[ENEMIES + 1] = enemy(7, 6);
    game.moveEnemies();
    CHECK(console.state[ENEMIES] == enemy(8, 3));
    CHECK(console.state[DIRECTIONS] & 1);

    reset();
    console.state[ENEMIES] = enemy(8, 5);
    console.state[ENEMIES + 1] = enemy(8, 6);
    game.doGravity();
    CHECK(console.state[ENEMIES] == enemy(8, 5));
    CHECK(console.state[ENEMIES + 1] == enemy(8, 6));
    game.moveEnemies(); // Enemies can support a patrol step, as before.
    CHECK(console.state[ENEMIES] == enemy(7, 5));

    reset();
    console.state[ENEMIES] = enemy(8, 7);
    console.state[ENEMIES + 1] = enemy(8, 0);
    game.doGravity();
    CHECK(!(console.state[ENEMIES] & 0x80)); // No wrap onto row-zero enemy.
}

static void scrollingAndSpawn() {
    reset();
    console.state[ENEMIES] = enemy(0, 6);
    console.state[ENEMIES + 1] = enemy(8, 6);
    console.state[DIRECTIONS] = 2;
    CHECK(!game.move(true));
    CHECK(!(console.state[ENEMIES] & 0x80));
    CHECK(console.state[ENEMIES + 1] == enemy(7, 6));
    CHECK(console.state[DIRECTIONS] == 2);
    console.state[ENEMIES] = enemy(15, 6);
    CHECK(!game.move(false));
    CHECK(!(console.state[ENEMIES] & 0x80));
    CHECK(console.state[ENEMIES + 1] == enemy(8, 6));
    CHECK(console.state[DIRECTIONS] == 2);

    for (uint8_t index = 0; index < 4; index++) {
        reset();
        console.state[X] = 9; // Next level column is 21, the first spawn.
        console.state[DIRECTIONS] = 0x0f;
        for (uint8_t slot = 0; slot < 4; slot++) {
            console.state[ENEMIES + slot] = enemy(10 + slot, 6);
        }
        console.state[ENEMIES + index] &= ~0x80;
        CHECK(!game.move(true));
        CHECK(console.state[ENEMIES + index] == enemy(15, 1));
        CHECK(console.state[DIRECTIONS] == (0x0f & ~(1 << index)));
    }
}

static void playerCollisions() {
    reset();
    console.state[ENEMIES] = enemy(5, 6);
    game.moveEnemies();
    CHECK(!(console.state[ENEMIES] & 0x80));
    CHECK(game.getLives() == 2);

    reset();
    console.state[ENEMIES] = enemy(3, 6);
    console.state[DIRECTIONS] = 1;
    game.moveEnemies();
    CHECK(!(console.state[ENEMIES] & 0x80));
    CHECK(game.getLives() == 2);

    reset();
    console.state[ENEMIES] = enemy(5, 6);
    CHECK(!game.move(true));
    CHECK(!(console.state[ENEMIES] & 0x80));
    CHECK(game.getLives() == 2);

    reset();
    console.state[Y] = (5 << 4) + 15;
    console.state[ENEMIES] = enemy(4, 6);
    game.doGravity();
    CHECK(!(console.state[ENEMIES] & 0x80));
    CHECK(game.getLives() == 3);
    CHECK(static_cast<int8_t>(console.state[V]) == -12);

    reset();
    console.state[ENEMIES] = enemy(4, 5);
    game.doGravity();
    CHECK(!(console.state[ENEMIES] & 0x80));
    CHECK(game.getLives() == 2);
}

static void timingRenderingAndRestart() {
    reset();
    console.state[ENEMIES] = enemy(10, 6);
    tick();
    CHECK(console.state[ENEMIES] == enemy(9, 6));
    CHECK(!console.getPixel(9, 6)); // Blink tick zero.
    tick();
    CHECK(console.getPixel(9, 6));
    CHECK(console.getPixel(4, 6));
    for (uint8_t i = 0; i < 14; i++) tick();
    CHECK(console.state[ENEMIES] == enemy(9, 6));
    tick(39);
    CHECK(console.state[ENEMIES] == enemy(9, 6));
    tick(1);
    CHECK(console.state[ENEMIES] == enemy(8, 6)); // 640 ms since first step.

    reset();
    console.state[TICK] = 4;
    buttonAdc = 850; // Right button through the real input accumulator.
    tick();
    CHECK(console.state[X] == 5);
    console.state[TICK] = 4;
    buttonAdc = 710; // Left.
    tick();
    CHECK(console.state[X] == 4);
    buttonAdc = 920; // Jump.
    tick();
    CHECK(static_cast<int8_t>(console.state[V]) < 0);

    console.state[DIRECTIONS] = 0x0f;
    game.drawLives(0);
    tick();
    CHECK(console.state[TICK] == 0x80);
    tick(999);
    CHECK(console.state[TICK] == 0x80);
    tick(1);
    CHECK(console.state[DIRECTIONS] == 0);
    CHECK(console.state[X] == 4 && console.state[Y] == 0);
    CHECK(game.getLives() == 3);
    for (uint8_t i = 0; i < 4; i++) CHECK(console.state[ENEMIES + i] == 0);
    CHECK(console.getPixel(4, 0));
    CHECK(console.getPixel(4, 7));

    reset();
    console.state[X] = 124; // Column 136 ends the fixed level.
    console.state[DIRECTIONS] = 0x0f;
    CHECK(game.move(true));
    CHECK(console.state[LEVEL] == 2);
    tick(1000);
    CHECK(console.state[DIRECTIONS] == 0);
    CHECK(console.state[TICK] == 0);

    reset();
    console.state[LEVEL] = 15;
    console.state[X] = 243;
    CHECK(game.move(true));
    CHECK(console.state[LEVEL] == 16);

    reset();
    console.state[LEVEL] = 16;
    console.state[X] = 243;
    CHECK(game.move(true));
    CHECK(console.state[LEVEL] == 1);

    console.state[DIRECTIONS] = 0x0f;
    game.begin();
    tick(1000);
    CHECK(console.state[LEVEL] == 1);
    CHECK(console.state[DIRECTIONS] == 0);
}

int main() {
    patrol();
    ledgesAndGravity();
    scrollingAndSpawn();
    playerCollisions();
    timingRenderingAndRestart();
    launcherFailureLine = checkLauncher();
    CHECK(launcherFailureLine == 0);
    completed = true;
    testFinished();
    for (;;) {}
}
