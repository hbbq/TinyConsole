#include "TinyConsole.h"

#include <avr/io.h>

namespace {
constexpr byte DATA_MASK = _BV(PB0);
constexpr byte CS_MASK = _BV(PB1);
constexpr byte CLK_MASK = _BV(PB2);
constexpr byte MATRIX_COUNT = 2;

// MAX7219 samples data on the rising clock edge, most significant bit first.
void sendByte(byte value) {
  for (byte bit = 0; bit < 8; bit++) {
    if (value & 0x80) PORTB |= DATA_MASK;
    else PORTB &= ~DATA_MASK;
    PORTB |= CLK_MASK;
    PORTB &= ~CLK_MASK;
    value <<= 1;
  }
}

void sendAll(byte reg, byte data) {
  PORTB &= ~CS_MASK;

  for (byte i = 0; i < MATRIX_COUNT; i++) {
    sendByte(reg);
    sendByte(data);
  }

  PORTB |= CS_MASK;
}
}

void TinyConsole::updateRow(byte row) {
  PORTB &= ~CS_MASK;

  // MAX7219 längst bort i kedjan får första datan.
  // screen[0] ska visas längst till vänster.
  for (byte matrix = 0; matrix < MATRIX_COUNT; matrix++) {
    #ifdef FLIPY
    sendByte(8 - row);
    #else
    sendByte(row + 1);
    #endif
    sendByte(screen[matrix][row]);
  }

  PORTB |= CS_MASK;
}

void TinyConsole::updateDisplay() {
  for (byte row = 0; row < 8; row++) {
    updateRow(row);
  }
}

void TinyConsole::setBrightness(byte level) {
  if (level > 15) level = 15;
  sendAll(0x0A, level);
}

void TinyConsole::begin(){
    // Set idle levels before enabling outputs; preserve the other port pins.
    PORTB |= CS_MASK;
    PORTB &= ~(DATA_MASK | CLK_MASK);
    DDRB |= DATA_MASK | CS_MASK | CLK_MASK;

    sendAll(0x0F, 0x00);
    sendAll(0x0C, 0x01);
    sendAll(0x09, 0x00);
    sendAll(0x0B, 0x07);
    sendAll(0x0A, 0x03);

    clearState();
    clearScreen();
}
