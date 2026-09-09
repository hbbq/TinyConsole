#include "Game.h"

#include "BreakoutApp.h"
#include "RacerApp.h"
#include "SkyHopApp.h"
#include "ShiftApp.h"
#include "SrbApp.h"

void Game::begin() {
    switch(id) {
        case GameId::Breakout:
            BreakoutApp(*console).begin();
            break;
        case GameId::SkyHop:
            SkyHopApp(*console).begin();
            break;
        case GameId::Shift:
            ShiftApp(*console).begin();
            break;
        case GameId::Srb:
            SrbApp(*console).begin();
            break;
        case GameId::Racer:
        default:
            RacerApp(*console).begin();
            break;
    }
}

void Game::update() {
    switch(id) {
        case GameId::Breakout:
            BreakoutApp(*console).update();
            break;
        case GameId::SkyHop:
            SkyHopApp(*console).update();
            break;
        case GameId::Shift:
            ShiftApp(*console).update();
            break;
        case GameId::Srb:
            SrbApp(*console).update();
            break;
        case GameId::Racer:
        default:
            RacerApp(*console).update();
            break;
    }
}
