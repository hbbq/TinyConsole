#include "RacerApp.h"
#include "TinyRandom.h"

enum StateSlot {
    PLAYERPOS_TRACKTOP = 0,
    TRACKWIDTH_CURVING = 1,
    STEPS = 2, // Two bytes: 2..3
    SPEED = 4, // In 25 ms units
    LAST_MV = 5,
};

void RacerApp::movePlayer(int8_t deltaY){
    uint8_t playerPos = TinyConsole::unpackHighNibble(console.state[PLAYERPOS_TRACKTOP]);
    uint8_t trackTop = TinyConsole::unpackLowNibble(console.state[PLAYERPOS_TRACKTOP]);
    playerPos += deltaY;
    if(playerPos < 0) playerPos = 0;
    if(playerPos >= console.Height) playerPos = console.Height - 1;
    console.state[PLAYERPOS_TRACKTOP] = TinyConsole::packNibbles(playerPos, trackTop);
}

void RacerApp::drawPlayer(){
    uint8_t playerPos = TinyConsole::unpackHighNibble(console.state[PLAYERPOS_TRACKTOP]);
    console.setPixel(0, playerPos);
}

void RacerApp::startLevel(){
    uint8_t playerPos = 4;
    uint8_t trackTop = 2;

    console.state[PLAYERPOS_TRACKTOP] = TinyConsole::packNibbles(playerPos, trackTop);
    console.setState16(STEPS, 0);
    console.state[LAST_MV] = 0;

    setLevel();
}

void RacerApp::setLevel(){
    uint16_t steps = console.getState16(STEPS);
    // Store speed in 25 ms units (2..20) to keep it in one byte.
    uint8_t speed = 2 + 1800 / (steps + 100);
    uint8_t trackWidth = 3 + 600 / (steps + 150);
    uint8_t curving = 2 + 1600 / (steps + 200);

    if(trackWidth > 7) trackWidth = 7;
    if(steps > 2000) trackWidth = 2;

    console.state[TRACKWIDTH_CURVING] = TinyConsole::packNibbles(trackWidth, curving);
    console.state[SPEED] = speed;
}

void RacerApp::moveTrack(){
    uint8_t lastMv = console.state[LAST_MV];
    uint8_t trackWidth = TinyConsole::unpackHighNibble(console.state[TRACKWIDTH_CURVING]);
    uint8_t curving = TinyConsole::unpackLowNibble(console.state[TRACKWIDTH_CURVING]);
    uint8_t mv = tinyRandom(curving);
    
    if(lastMv + mv == 1) mv = 2;

    console.state[LAST_MV] = mv;

    uint8_t playerPos = TinyConsole::unpackHighNibble(console.state[PLAYERPOS_TRACKTOP]);
    uint8_t trackTop = TinyConsole::unpackLowNibble(console.state[PLAYERPOS_TRACKTOP]);

    if (mv == 0) trackTop--;
    if (mv == 1) trackTop++;

    if(trackTop < 0) trackTop = 0;
    if(trackTop + trackWidth >= console.Height) trackTop = console.Height - trackWidth;
    
    console.state[PLAYERPOS_TRACKTOP] = TinyConsole::packNibbles(playerPos, trackTop);

    console.shiftLeft();
    for(int y = 0; y < console.Height; y++){
        if(y < trackTop || y >= trackTop + trackWidth) console.setPixel(console.Width - 1, y, true);
    }

    uint16_t steps = console.getState16(STEPS);
    if(steps < 60000) console.setState16(STEPS, steps + 1);

    setLevel();
}

void RacerApp::begin(){
    startLevel();
}

void RacerApp::update(){
    console.update();

    if(!console.tickDue(static_cast<uint16_t>(console.state[SPEED]) * 25)) return;

    uint8_t buttons = console.consumeButtons();

    if (buttons & BTN_UP) movePlayer(-1);
    if (buttons & BTN_DOWN) movePlayer(1);

    uint8_t playerPos = TinyConsole::unpackHighNibble(console.state[PLAYERPOS_TRACKTOP]);

    bool collision = console.getPixel(1, playerPos);

    if(collision){
        /*
        for(int i = 0; i < 5; i++){
            console.setPixel(0, playerPos, false);
            console.updateDisplay();
            delay(300);
            console.setPixel(0, playerPos, true);
            console.updateDisplay();
            delay(300);
        }
        */
        console.clearScreen();
        startLevel();
        return;
    }

    moveTrack();

    drawPlayer();
}
