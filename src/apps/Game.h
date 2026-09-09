#pragma once

#include <Arduino.h>

class TinyConsoleGameApi;

enum class GameId : uint8_t {
    Racer,
    Breakout,
    SkyHop,
    Shift,
    Srb,
    Count
};

constexpr uint8_t gameCount = static_cast<uint8_t>(GameId::Count);

class Game {
public:
    Game(TinyConsoleGameApi& console, GameId id)
        : console(&console), id(id) {}

    void begin();
    void update();

private:
    TinyConsoleGameApi* console;
    GameId id;
};
