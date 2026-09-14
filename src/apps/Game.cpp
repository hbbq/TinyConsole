#include "Game.h"

#if ENABLE_BREAKOUT
#include "BreakoutApp.h"
#endif
#if ENABLE_RACER
#include "RacerApp.h"
#endif
#if ENABLE_SKYHOP
#include "SkyHopApp.h"
#endif
#if ENABLE_SHIFT
#include "ShiftApp.h"
#endif
#if ENABLE_SRB
#include "SrbApp.h"
#endif

void Game::begin() {
    switch(id) {
#if !ENABLE_RACER
        default:
#endif
#if ENABLE_BREAKOUT
        case GameId::Breakout:
            BreakoutApp(*console).begin();
            break;
#endif
#if ENABLE_SKYHOP
        case GameId::SkyHop:
            SkyHopApp(*console).begin();
            break;
#endif
#if ENABLE_SHIFT
        case GameId::Shift:
            ShiftApp(*console).begin();
            break;
#endif
#if ENABLE_SRB
        case GameId::Srb:
            SrbApp(*console).begin();
            break;
#endif
#if ENABLE_RACER
        case GameId::Racer:
        default:
            RacerApp(*console).begin();
            break;
#endif
    }
}

void Game::update() {
    switch(id) {
#if !ENABLE_RACER
        default:
#endif
#if ENABLE_BREAKOUT
        case GameId::Breakout:
            BreakoutApp(*console).update();
            break;
#endif
#if ENABLE_SKYHOP
        case GameId::SkyHop:
            SkyHopApp(*console).update();
            break;
#endif
#if ENABLE_SHIFT
        case GameId::Shift:
            ShiftApp(*console).update();
            break;
#endif
#if ENABLE_SRB
        case GameId::Srb:
            SrbApp(*console).update();
            break;
#endif
#if ENABLE_RACER
        case GameId::Racer:
        default:
            RacerApp(*console).update();
            break;
#endif
    }
}
