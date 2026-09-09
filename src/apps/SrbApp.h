#pragma once

#include "hardware/TinyConsoleGameApi.h"

class SrbApp {
public:
    SrbApp(TinyConsoleGameApi& console)
        : console(console) {}

    void begin();
    void update();

    uint8_t getLevelColumn(uint8_t x);

    void startLevel(bool newLevel = true);
    // Returns true when movement starts the next level.
    bool move(bool right);
    void drawPlayer(bool draw);
    void drawEnemies(bool draw);
    void doGravity();
    void doJump();
    void moveEnemies();

    uint8_t getLives();
    void drawLives(uint8_t lives);

private:
    void enterLevel();
    bool enemyBlocked(uint8_t x, uint8_t y) const;
    void checkEnemyCollision(uint8_t index, bool hurtPlayer);
    TinyConsoleGameApi& console;
};
