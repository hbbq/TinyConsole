#include "appConf.h"

#ifdef USE_SMALLNUMBERTEXT

#include "SmallNumberText.h"

#include <Arduino.h>

struct SmallTextGlyph {
  char c;
  byte columns[4];
};

const SmallTextGlyph SMALLTEXTFONT[] PROGMEM = {
  { ' ', {
    B000000,
    B000000,
    B000000,
    B000000
  }},
  { '0', {
    B011110,
    B100001,
    B100001,
    B011110
  }},
  { '1', {
    B100000,
    B100010,
    B111111,
    B100000
  }},
  { '2', {
    B111001,
    B100101,
    B100101,
    B100010
  }},
  { '3', {
    B100001,
    B100101,
    B100101,
    B011010
  }},
  { '4', {
    B000111,
    B000100,
    B000100,
    B111111
  }},
  { '5', {
    B100111,
    B100101,
    B100101,
    B011001
  }},
  { '6', {
    B011110,
    B100101,
    B100101,
    B011001
  }},
  { '7', {
    B000001,
    B110001,
    B001101,
    B000011
  }},
  { '8', {
    B011010,
    B100101,
    B100101,
    B011010
  }},
  { '9', {
    B100010,
    B100101,
    B100101,
    B011110
  }},
};

const byte SMALLTEXTGLYPH_COUNT = sizeof(SMALLTEXTFONT) / sizeof(SMALLTEXTFONT[0]);

byte findSmallTextGlyph(char c) {
  for (byte i = 0; i < SMALLTEXTGLYPH_COUNT; i++) {
    if (pgm_read_byte(&SMALLTEXTFONT[i].c) == c) {
      return i;
    }
  }

  return 0;
}

void SmallNumberText::drawText(TinyConsole * console, int x, int y, const char* text){
    for(uint8_t i = 0; i < strlen(text); i++){
        uint8_t xp = x + i * 5;

        char c = text[i];

        byte index = findSmallTextGlyph(c);

        for(uint8_t bx = 0; bx < 4; bx++){
            byte column = pgm_read_byte(&SMALLTEXTFONT[index].columns[bx]);
            
            for(uint8_t by = 0; by < 6; by++){
                bool on = (column >> by) & B1;

                console->setPixel(xp+bx, y+by, on);
            }
        }
    }
}

#endif