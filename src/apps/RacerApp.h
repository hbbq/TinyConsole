#pragma once

#include "hardware/TinyConsole.h"

class RacerApp {
public:
    RacerApp(TinyConsoleGameApi& console)
        : console(console) {}

    void begin();
    void update();

private:
    TinyConsoleGameApi& console;
    
    void startLevel();
    void setLevel();

    void movePlayer(int8_t deltaY);
    void drawPlayer();

    void moveTrack();
};
