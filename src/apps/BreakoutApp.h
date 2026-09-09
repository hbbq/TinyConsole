#pragma once

#include "hardware/TinyConsole.h"

class BreakoutApp {
public:
    BreakoutApp(TinyConsoleGameApi& console)
        : console(console) {}

    void begin();
    void update();

private:
    TinyConsoleGameApi& console;
    
    void startLevel();

    void movePlayer(int deltaY);
    void drawPlayer();

    void moveBall();
    void bounceOrBreak(uint8_t& nextX, int8_t& nextY, int8_t& ballSX, int8_t& ballSY);
    void advanceBricks();
    void clearBall();
    void drawBall();
    int8_t sxBitToSx(bool sxBit){
        return sxBit == true ? 1 : -1;
    }
    bool sxToSxBit(int8_t sx){
        return sx == 1 ? true : false;
    }
};
