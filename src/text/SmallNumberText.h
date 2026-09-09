#pragma once

#include "appConf.h"

#ifdef USE_SMALLNUMBERTEXT

#include "hardware/TinyConsole.h"

class SmallNumberText{
    public:
        void drawText(TinyConsole * console, int x, int y, const char* text);
};

#endif
