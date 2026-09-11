#pragma once

#include <Arduino.h>
#include "appConf.h"

class TinyConsoleGameApi;

enum class GameId : uint8_t {
#if ENABLE_RACER
    Racer,
#endif
#if ENABLE_BREAKOUT
    Breakout,
#endif
#if ENABLE_SKYHOP
    SkyHop,
#endif
#if ENABLE_SHIFT
    Shift,
#endif
#if ENABLE_SRB
    Srb,
#endif
    Count
};

constexpr uint8_t gameCount = static_cast<uint8_t>(GameId::Count);

class Game {
public:
    constexpr Game(TinyConsoleGameApi& console, GameId id)
        : console(&console), id(id) {}

    void begin();
    void update();

private:
    TinyConsoleGameApi* console;
    GameId id;
};
