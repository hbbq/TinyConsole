#include "GameFactory.h"

Game GameFactory::get(TinyConsole& console, GameId id) {
    return Game(console, id);
}
