#pragma once

#include "Game.h"
#include "hardware/TinyConsole.h"

class GameFactory {
public:
    static Game get(TinyConsole& console, GameId id);
};
