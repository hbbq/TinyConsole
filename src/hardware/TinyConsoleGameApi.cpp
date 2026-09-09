#include "TinyConsoleGameApi.h"

#define BTN 3   // PB3

constexpr byte MATRIX_COUNT = 2;

void TinyConsoleGameApi::clearScreen() {
  for (byte m = 0; m < MATRIX_COUNT; m++) {
    for (byte y = 0; y < 8; y++) {
      screen[m][y] = 0;
    }
  }
}

void TinyConsoleGameApi::setPixel(uint8_t x, uint8_t y, bool on) {
  if (x >= Width || y >= Height) return;

  byte matrix = x / 8;
  byte localX = x % 8;

  // MAX7219 register 1..8 = rader
  byte physicalRow = y;

  // En bit i raden = X-position
  byte mask = 1 << localX;

  if (on) {
    screen[matrix][physicalRow] |= mask;
  } else {
    screen[matrix][physicalRow] &= ~mask;
  }
}

bool TinyConsoleGameApi::getPixel(uint8_t x, uint8_t y) const {
  if (x >= Width || y >= Height) return false;

  byte matrix = x / 8;
  byte localX = x % 8;

  byte mask = 1 << localX;

  return (screen[matrix][y] & mask) != 0;
}

void TinyConsoleGameApi::shiftLeft(){
  for (byte row = 0; row < 8; row++) {
    for (byte matrix = 0; matrix < MATRIX_COUNT; matrix++) {

      // Pixeln längst till vänster i nästa display
      // ska flytta in längst till höger i denna.
      byte carry = 0;

      if (matrix < MATRIX_COUNT - 1) {
        carry = (screen[matrix + 1][row] & 0x01) << 7;
      }

      screen[matrix][row] =
        (screen[matrix][row] >> 1) | carry;
    }
  }
}

void TinyConsoleGameApi::shiftRight(){
  for (byte row = 0; row < 8; row++) {
    for (byte matrix = MATRIX_COUNT; matrix-- > 0;) {
      // The previous display's rightmost pixel enters at the left.
      byte carry = 0;
      if (matrix > 0) {
        carry = (screen[matrix - 1][row] & 0x80) >> 7;
      }
      screen[matrix][row] = (screen[matrix][row] << 1) | carry;
    }
  }
}

void TinyConsoleGameApi::insertColumn(uint8_t x, byte column){
  for (byte y = 0; y < 8; y++) {
    setPixel(x, y, column & 1);
    column >>= 1;
  }
}

uint8_t TinyConsoleGameApi::readButtons() {
    int value = analogRead(BTN);

    if (value < 600) return BTN_UP | BTN_DOWN | BTN_ACTION;
    if (value < 642) return BTN_UP | BTN_DOWN;
    if (value < 689) return BTN_UP | BTN_ACTION;
    if (value < 741) return BTN_UP;
    if (value < 804) return BTN_DOWN | BTN_ACTION;
    if (value < 881) return BTN_DOWN;
    if (value < 973) return BTN_ACTION;

    return 0;
}

void TinyConsoleGameApi::update() {
    accumulatedButtons |= readButtons();
}

bool TinyConsoleGameApi::tickDue(uint16_t intervalMs) {
    if (millis() - lastTick < intervalMs) {
        return false;
    }

    lastTick = millis();
    return true;
}

uint8_t TinyConsoleGameApi::consumeButtons() {
    uint8_t result = accumulatedButtons;
    accumulatedButtons = 0;
    return result;
}

void TinyConsoleGameApi::showNumber(uint16_t value) {
    static const uint8_t digits[10][3] PROGMEM = {
        { B11111, B10001, B11111 },
        { B00000, B11111, B00000 },
        { B11101, B10101, B10111 },
        { B10101, B10101, B11111 },
        { B00111, B00100, B11111 },
        { B10111, B10101, B11101 },
        { B11111, B10101, B11101 },
        { B00001, B00001, B11111 },
        { B11111, B10101, B11111 },
        { B10111, B10101, B11111 }
    };

    if (value > 9999) value = 9999;
    // Rightmost digit's position: 3-pixel glyphs with 1-pixel spacing.
    uint8_t x = value < 10 ? 6 : value < 100 ? 8 : value < 1000 ? 10 : 12;
    clearScreen();
    do {
        uint8_t digit = value % 10;
        for (uint8_t column = 0; column < 3; column++) {
            insertColumn(x + column, pgm_read_byte(&digits[digit][column]) << 1);
        }
        x -= 4;
        value /= 10;
    } while (value);
}

void TinyConsoleGameApi::showCountdown(uint8_t digit) {
  static const uint8_t digits[][4] PROGMEM = {
    { B100000, B100010, B111111, B100000 },
    { B111001, B100101, B100101, B100010 },
    { B100001, B100101, B100101, B011010 }
  };

    clearScreen();
    for (uint8_t x = 0; x < 4; x++) {
      insertColumn(6 + x, pgm_read_byte(&digits[digit - 1][x]) << 1);
    }
}

void TinyConsoleGameApi::clearState() {
    for (uint8_t i = 0; i < STATE_SIZE; i++) {
        state[i] = 0;
    }
}
