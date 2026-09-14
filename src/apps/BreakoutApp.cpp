#include "BreakoutApp.h"
#include "TinyRandom.h"

constexpr int playerSize = 7;
constexpr int playerPad = 3; // ((playerSize - 1) / 2);

constexpr uint8_t ballYScale = 16;
constexpr uint8_t ballYShift = 4;

enum StateSlot{
    BALLX_PLAYERY_BALLSX = 0,
    BALLY = 1,
    BALLSY = 2,
    EXPLOSIVE_BRICK = 3
};

// Explosive brick position is packed as x * 8 + y; x = 0 is the paddle.

void BreakoutApp::movePlayer(int deltaY){
  uint8_t ballX = TinyConsole::unpackX(console.state[BALLX_PLAYERY_BALLSX]);
  uint8_t playerY = TinyConsole::unpackY(console.state[BALLX_PLAYERY_BALLSX]);
  bool sxBit = TinyConsole::unpackBool(console.state[BALLX_PLAYERY_BALLSX]);
  playerY += deltaY;
  if(playerY - playerPad < 0) playerY = playerPad;
  if(playerY + playerPad >= console.Height) playerY = console.Height - playerPad - 1;
  console.state[BALLX_PLAYERY_BALLSX] = TinyConsole::packXYB(ballX, playerY, sxBit);;
}

void BreakoutApp::drawPlayer(){
  uint8_t playerY = TinyConsole::unpackY(console.state[BALLX_PLAYERY_BALLSX]);
  for(int y = 0; y < console.Height; y++){
    console.setPixel(0, y, y >= playerY - playerPad && y <= playerY + playerPad);
  }
}

void BreakoutApp::moveBall(){
    uint8_t ballX = TinyConsole::unpackX(console.state[BALLX_PLAYERY_BALLSX]);
    uint8_t playerY = TinyConsole::unpackY(console.state[BALLX_PLAYERY_BALLSX]);
    int8_t ballSX = sxBitToSx(TinyConsole::unpackBool(console.state[BALLX_PLAYERY_BALLSX]));
    int8_t ballY = console.state[BALLY];
    int8_t ballSY = console.state[BALLSY];
    uint8_t nextX = ballX + ballSX;
    int8_t nextY = ballY + ballSY;

    if(nextY < 0 || nextY >= console.Height * ballYScale){
        ballSY *= -1;
        nextY = ballY + ballSY;
    }

    if(nextX >= console.Width){
        ballSX = -1;
        nextX = ballX + ballSX;
    }else if(nextX <= 0){
        int16_t nextPixelY = static_cast<uint8_t>(nextY) >> ballYShift;
        if(nextPixelY >= playerY - playerPad &&
           nextPixelY <= playerY + playerPad){
            ballSX = 1;
            ballSY = static_cast<int16_t>((tinyRandom(7) * ballYScale) / 3U) - ballYScale;
        }else{
            advanceBricks();
            ballSX = 1;
            ballSY = 0;
        }

        nextX = ballX + ballSX;
        nextY = ballY + ballSY;
    }

    bounceOrBreak(nextX, nextY, ballSX, ballSY);

    ballX = nextX;
    ballY = nextY;

    console.state[BALLX_PLAYERY_BALLSX] = TinyConsole::packXYB(ballX, playerY, sxToSxBit(ballSX));;
    console.state[BALLY] = ballY;
    console.state[BALLSY] = ballSY;
}

void BreakoutApp::bounceOrBreak(uint8_t& nextX, int8_t& nextY, int8_t& ballSX, int8_t& ballSY){
    uint8_t ballX = TinyConsole::unpackX(console.state[BALLX_PLAYERY_BALLSX]);
    int8_t ballY = console.state[BALLY];
    uint8_t currentX = ballX;
    int8_t currentY = static_cast<uint8_t>(ballY) >> ballYShift;
    uint8_t nextPixelX = nextX;
    int8_t nextPixelY = static_cast<uint8_t>(nextY) >> ballYShift;
    bool movedHorizontally = nextPixelX != currentX;
    bool movedVertically = nextPixelY != currentY;
    bool horizontalBrick = movedHorizontally &&
                           hasBrick(nextPixelX, currentY);
    bool verticalBrick = movedVertically &&
                         hasBrick(currentX, nextPixelY);
    uint8_t hitX;
    uint8_t hitY;

    if(horizontalBrick && verticalBrick){
        int16_t horizontalSpeed = (ballSX < 0 ? -ballSX : ballSX) * ballYScale;
        int16_t verticalSpeed = ballSY < 0 ? -ballSY : ballSY;

        if(horizontalSpeed >= verticalSpeed){
            hitX = nextPixelX;
            hitY = currentY;
            ballSX *= -1;
        }else{
            hitX = currentX;
            hitY = nextPixelY;
            ballSY *= -1;
        }
    }else if(horizontalBrick){
        hitX = nextPixelX;
        hitY = currentY;
        ballSX *= -1;
    }else if(verticalBrick){
        hitX = currentX;
        hitY = nextPixelY;
        ballSY *= -1;
    }else if(hasBrick(nextPixelX, nextPixelY)){
        hitX = nextPixelX;
        hitY = nextPixelY;

        if(movedVertically && !movedHorizontally){
            ballSY *= -1;
        }else if(movedHorizontally && !movedVertically){
            ballSX *= -1;
        }else{
            ballSX *= -1;
            ballSY *= -1;
        }
    }else{
        return;
    }

    breakBrick(hitX, hitY);
    nextX = ballX + ballSX;
    nextY = ballY + ballSY;
}

bool BreakoutApp::hasBrick(uint8_t x, uint8_t y){
    uint8_t explosive = console.state[EXPLOSIVE_BRICK];
    return console.getPixel(x, y) || explosive == (x << 3) + y;
}

void BreakoutApp::breakBrick(uint8_t x, uint8_t y){
    uint8_t explosive = console.state[EXPLOSIVE_BRICK];
    if(explosive != (x << 3) + y){
        console.setPixel(x, y, false);
        return;
    }

    uint8_t firstX = x - (x > 1);
    uint8_t lastX = x + (x < console.Width - 1);
    uint8_t firstY = y - (y > 0);
    uint8_t lastY = y + (y < console.Height - 1);
    for(uint8_t blastX = firstX; blastX <= lastX; blastX++){
        for(uint8_t blastY = firstY; blastY <= lastY; blastY++){
            console.setPixel(blastX, blastY, false);
        }
    }
    uint8_t nextY = (y + 4) & 7;
    console.state[EXPLOSIVE_BRICK] = (15 << 3) + nextY;
}

void BreakoutApp::blinkExplosiveBrick(){
    uint8_t explosive = console.state[EXPLOSIVE_BRICK];
    console.setPixel(explosive >> 3, explosive & 7,
                     console.state[BALLX_PLAYERY_BALLSX] & 0x10);
}

void BreakoutApp::advanceBricks(){
    console.shiftLeft();
    console.insertColumn(console.Width - 1, B11111111);

    uint8_t& explosive = console.state[EXPLOSIVE_BRICK];
    if(explosive >= 0x10){
        explosive -= 8;
    }else{
        explosive += 14 << 3;
    }
}

void BreakoutApp::clearBall(){
    uint8_t ballX = TinyConsole::unpackX(console.state[BALLX_PLAYERY_BALLSX]);
    int8_t ballY = console.state[BALLY];
    console.setPixel(ballX, static_cast<uint8_t>(ballY) >> ballYShift, false);
}

void BreakoutApp::drawBall(){
    uint8_t ballX = TinyConsole::unpackX(console.state[BALLX_PLAYERY_BALLSX]);
    int8_t ballY = console.state[BALLY];
  console.setPixel(ballX, static_cast<uint8_t>(ballY) >> ballYShift, true);
}

void BreakoutApp::startLevel(){
    for(int x = console.Width / 2; x < console.Width; x++){
        for(int y = 0; y < console.Height; y++){
            console.setPixel(x,y,true);
        }
    }
    uint8_t playerY = console.Height / 2;
    uint8_t ballX = 1;
    int8_t ballY = playerY * ballYScale;
    uint8_t ballSX = 1;
    int8_t ballSY = 0;

    console.state[BALLX_PLAYERY_BALLSX] = TinyConsole::packXYB(ballX, playerY, sxToSxBit(ballSX));;
    console.state[BALLY] = ballY;
    console.state[BALLSY] = ballSY;
    console.state[EXPLOSIVE_BRICK] = (12 << 3) + 3;
}

void BreakoutApp::begin(){
    startLevel();
}

void BreakoutApp::update(){    
    console.update();

    if (!console.tickDue(100)) return;

    uint8_t buttons = console.consumeButtons();

    // Kör nästa frame
    if (buttons & BTN_UP) movePlayer(-1);
    if (buttons & BTN_DOWN) movePlayer(1);

    drawPlayer();

    clearBall();
    moveBall();
    drawBall();
    blinkExplosiveBrick();
}
