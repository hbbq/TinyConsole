#pragma once

#include "hardware/TinyConsoleGameApi.h"

class ShiftApp {
public:
    ShiftApp(TinyConsoleGameApi& console)
        : console(console) {}
    
    uint8_t getColumn(uint8_t x);
    void createBoard();
    void performBlink();

    uint8_t getState();
    void setState(uint8_t value);

    bool getRamBit(uint8_t x, uint8_t y);
    void setRamBit(uint8_t x, uint8_t y, bool value);

    void moveMarker(bool up);
    void shiftRow();
    bool matchPixels();
    bool collapseDown();
    bool collapseLeft();
    bool columnIsEmpty(uint8_t x);
    void moveColumn(uint8_t from, uint8_t to);
    void clearColumn(uint8_t x);

    void begin();
    void update();

private:
    TinyConsoleGameApi& console;
};
