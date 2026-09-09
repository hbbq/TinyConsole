#pragma once

#include "hardware/TinyConsole.h"

class SkyHopApp {
public:
    SkyHopApp(TinyConsoleGameApi& console)
        : console(console) {}

    void begin();
    void update();

private:
    TinyConsoleGameApi& console;

    void startRun();
    void moveObstacles();
    void endRun();
    bool columnOccupied(uint8_t x) const;
};
