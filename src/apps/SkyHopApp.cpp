#include "SkyHopApp.h"
#include "TinyRandom.h"

namespace {
constexpr uint8_t playerX = 2;
constexpr uint8_t fixedPointShift = 4;
constexpr uint8_t gapHeight = 4;
constexpr uint8_t obstacleSpacing = 8;
constexpr uint8_t gameOverFlag = 0x80;
constexpr uint8_t phaseMask = 0x07;
constexpr uint16_t frameTimeMs = 125;
constexpr int8_t flapVelocity = -10;
constexpr int8_t gravity = 3;
constexpr int8_t maximumFallVelocity = 8;

enum StateSlot {
    PLAYER_Y = 0,
    PLAYER_VELOCITY = 1,
    FLAGS_PHASE = 2,
    SCORE = 3
};

}

void SkyHopApp::startRun() {
    console.clearState();
    console.clearScreen();
    console.state[PLAYER_Y] = (console.Height / 2) << fixedPointShift;
    console.setPixel(playerX, console.Height / 2);
}

bool SkyHopApp::columnOccupied(uint8_t x) const {
    for(uint8_t y = 0; y < console.Height; y++) {
        if(console.getPixel(x, y)) return true;
    }
    return false;
}

void SkyHopApp::moveObstacles() {
    if(columnOccupied(playerX) && console.state[SCORE] < 255) {
        console.state[SCORE]++;
    }

    console.shiftLeft();

    uint8_t phase = console.state[FLAGS_PHASE] & phaseMask;
    if(phase == 0) {
        uint8_t gapTop = tinyRandom(console.Height - gapHeight + 1);
        uint8_t column = static_cast<uint8_t>(~(((1U << gapHeight) - 1) << gapTop));
        console.insertColumn(console.Width - 1, column);
        phase = obstacleSpacing - 1;
    } else {
        phase--;
        console.insertColumn(console.Width - 1, 0);
    }
    console.state[FLAGS_PHASE] = phase;
}

void SkyHopApp::endRun() {
    console.state[FLAGS_PHASE] |= gameOverFlag;
    console.showNumber(console.state[SCORE]);
}

void SkyHopApp::begin() {
    startRun();
}

void SkyHopApp::update() {
    console.update();
    if(!console.tickDue(frameTimeMs)) return;

    uint8_t buttons = console.consumeButtons();
    if(console.state[FLAGS_PHASE] & gameOverFlag) {
        if(buttons & BTN_ACTION) startRun();
        return;
    }

    uint8_t oldPixelY = console.state[PLAYER_Y] >> fixedPointShift;
    console.setPixel(playerX, oldPixelY, false);

    int8_t velocity = static_cast<int8_t>(console.state[PLAYER_VELOCITY]);
    if(buttons & BTN_ACTION) {
        velocity = flapVelocity;
    } else if(velocity < maximumFallVelocity) {
        velocity += gravity;
    }

    int16_t nextY = static_cast<int16_t>(console.state[PLAYER_Y]) + velocity;
    moveObstacles();

    if(nextY < 0 || nextY >= static_cast<int16_t>(console.Height << fixedPointShift)) {
        endRun();
        return;
    }

    uint8_t pixelY = static_cast<uint8_t>(nextY) >> fixedPointShift;
    if(console.getPixel(playerX, pixelY)) {
        endRun();
        return;
    }

    console.state[PLAYER_Y] = static_cast<uint8_t>(nextY);
    console.state[PLAYER_VELOCITY] = static_cast<uint8_t>(velocity);
    console.setPixel(playerX, pixelY);
}
