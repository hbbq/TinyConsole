#pragma once

#include <Arduino.h>

#define BTN_UP     0b001
#define BTN_DOWN   0b010
#define BTN_ACTION 0b100

class TinyConsoleGameApi {
public:
    void update();

    bool tickDue(uint16_t intervalMs);
    uint8_t consumeButtons();

    uint8_t readButtons();

    void clearScreen();
    // Clear and draw 1..4 centered digits; values above 9999 are clamped.
    void showNumber(uint16_t value);
    void showCountdown(uint8_t digit);
    void setPixel(uint8_t x, uint8_t y, bool on = true);
    bool getPixel(uint8_t x, uint8_t y) const;

    void shiftLeft();
    void shiftRight();
    void insertColumn(uint8_t x, byte column);

    //void tone(uint16_t frequency);
    //void noTone();

    static constexpr uint8_t Width = 16;
    static constexpr uint8_t Height = 8;

    // Both applications currently use state slots 0..4.
    static constexpr uint8_t STATE_SIZE = 16;

    uint8_t state[STATE_SIZE];

    void clearState();

     // 4 bit X + 3 bit Y + 1 bool
    static uint8_t packXYB(uint8_t x, uint8_t y, bool flag) {
        return ((x & 0x0F) << 4)
             | ((y & 0x07) << 1)
             | (flag ? 1 : 0);
    }

    static uint8_t unpackX(uint8_t value) {
        return (value >> 4) & 0x0F;
    }

    static uint8_t unpackY(uint8_t value) {
        return (value >> 1) & 0x07;
    }

    static bool unpackBool(uint8_t value) {
        return value & 0x01;
    }

    // Två 4-bitars värden
    static uint8_t packNibbles(uint8_t high, uint8_t low) {
        return ((high & 0x0F) << 4)
             | (low & 0x0F);
    }

    static uint8_t unpackHighNibble(uint8_t value) {
        return (value >> 4) & 0x0F;
    }

    static uint8_t unpackLowNibble(uint8_t value) {
        return value & 0x0F;
    }

    void setState16(uint8_t index, uint16_t value) {
        state[index]     = static_cast<uint8_t>(value);
        state[index + 1] = static_cast<uint8_t>(value >> 8);
    }

    uint16_t getState16(uint8_t index) const {
        return static_cast<uint16_t>(state[index])
            | (static_cast<uint16_t>(state[index + 1]) << 8);
    }

    bool getBitInByte(uint8_t byte, uint8_t bitIndex) const {
        return (byte & (1 << bitIndex)) != 0;
    }

    uint8_t setBitInByte(uint8_t byte, uint8_t bitIndex, bool value) {
        if (value) {
            return byte | (1 << bitIndex);
        } else {
            return byte & ~(1 << bitIndex);
        }
    }

private:
    uint8_t accumulatedButtons = 0;
    unsigned long lastTick = 0;

protected:
    uint8_t screen[2][8];
};
