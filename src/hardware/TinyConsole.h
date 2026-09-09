#pragma once

#include "TinyConsoleGameApi.h"

class TinyConsole : public TinyConsoleGameApi {
public:
    void begin();
    void updateDisplay();
    void setBrightness(byte level);

    // system-only stuff
private:  
    void updateRow(byte row);
};