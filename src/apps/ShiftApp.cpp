#include "ShiftApp.h"
#include "TinyRandom.h"

constexpr uint8_t boardwidth = 14;
constexpr uint8_t boardOffset = 2;
constexpr uint8_t MATCH_SIZE = 3;

uint8_t ShiftApp::getColumn(uint8_t x) {
    uint8_t column = 0;

    for (uint8_t y = 8; y-- > 0;) {
        column = (column << 1) | console.getPixel(x, y);
    }

    return column;
}

uint8_t setBitsInByte(uint8_t byte, uint8_t mask, uint8_t value) {
    return (byte & ~mask) | (value & mask);
}

uint8_t ShiftApp::getState() {
    return console.state[0] & B00000001;
}

void ShiftApp::setState(uint8_t value) {
    console.state[0] = setBitsInByte(console.state[0], B00000001, value);
}

bool ShiftApp::getRamBit(uint8_t x, uint8_t y) {
    return console.getBitInByte(console.state[x], y);
}

void ShiftApp::setRamBit(uint8_t x, uint8_t y, bool value) {
    console.state[x] = console.setBitInByte(console.state[x], y, value);
}

void ShiftApp::createBoard() {
    for(uint8_t x = 0; x < boardwidth; x++) {
        console.insertColumn(x + boardOffset, 0xff);
        console.state[x + boardOffset] = tinyRandom(256) & tinyRandom(256) & tinyRandom(256);
    }
    console.insertColumn(0, B11111111);
    console.insertColumn(1, B00000001);
    console.state[0] = B00000000;
}
    
void ShiftApp::performBlink() {
    for(uint8_t x = 0; x < boardwidth; x++) {
        uint8_t column = getColumn(x + boardOffset);
        column ^= console.state[x + boardOffset];
        console.insertColumn(x + boardOffset, column);
    }
}

void ShiftApp::moveMarker(bool up) {
    uint8_t b = getColumn(1);
    b = up 
        ? b != B00000001 ? (b >> 1) : b
        : b != B10000000 ? (b << 1) : b;
    console.insertColumn(1, b);
}

void ShiftApp::shiftRow() {
    console.insertColumn(0, getColumn(0) << 1);
    for(uint8_t y = 0; y < 8; y++) {
        if (console.getPixel(1, y)) {
            bool leftmost = console.getPixel(boardOffset, y);
            for(uint8_t x = 0; x < boardwidth - 1; x++) {
                console.setPixel(x + boardOffset, y, console.getPixel(x + boardOffset + 1, y));                
            }
            console.setPixel(boardOffset + boardwidth - 1, y, leftmost);

            uint8_t mask = 1 << y;
            bool first = console.state[boardOffset] & mask;

            for (uint8_t x = 0; x < boardwidth - 1; x++) {
                if (console.state[boardOffset + x + 1] & mask)
                    console.state[boardOffset + x] |= mask;
                else
                    console.state[boardOffset + x] &= ~mask;
            }

            if (first)
                console.state[boardOffset + boardwidth - 1] |= mask;
            else
                console.state[boardOffset + boardwidth - 1] &= ~mask;

            break;
        }
    }
    setState(B01);
}

bool ShiftApp::matchPixels() {
    uint8_t remaining[boardwidth];
    uint8_t group[boardwidth] = {};
    uint8_t matches[boardwidth] = {};

    // Kopiera alla blinkpixlar till transient arbetsyta
    for (uint8_t x = 0; x < boardwidth; x++) {
        remaining[x] = console.state[boardOffset + x];
    }

    while (true) {
        // Hitta första kvarvarande blinkpixel
        int8_t startX = -1;
        uint8_t startBit = 0;

        for (uint8_t x = 0; x < boardwidth; x++) {
            if (remaining[x]) {
                startX = x;

                // välj lägsta satta bit
                startBit = remaining[x] & -remaining[x];
                break;
            }
        }

        if (startX < 0) {
            break; // inga fler grupper
        }

        // Ny grupp
        for (uint8_t x = 0; x < boardwidth; x++) {
            group[x] = 0;
        }

        group[startX] = startBit;

        // Expandera gruppen tills inget mer hittas
        bool changed;

        do {
            changed = false;

            for (uint8_t x = 0; x < boardwidth; x++) {
                
                // upp / ner inom kolumnen
                uint8_t expanded =
                    group[x]
                    | (group[x] << 1)
                    | (group[x] >> 1);

                expanded &= remaining[x];

                if (expanded != group[x]) {
                    group[x] = expanded;
                    changed = true;
                }

                // vänster
                if (x > 0) {
                    uint8_t add = group[x] & remaining[x - 1];
                    uint8_t newValue = group[x - 1] | add;

                    if (newValue != group[x - 1]) {
                        group[x - 1] = newValue;
                        changed = true;
                    }
                }

                // höger
                if (x + 1 < boardwidth) {
                    uint8_t add = group[x] & remaining[x + 1];
                    uint8_t newValue = group[x + 1] | add;

                    if (newValue != group[x + 1]) {
                        group[x + 1] = newValue;
                        changed = true;
                    }
                }
            }
        } while (changed);

        // Räkna pixlar i gruppen
        uint8_t count = 0;

        for (uint8_t x = 0; x < boardwidth; x++) {
            uint8_t bits = group[x];

            while (bits) {
                count += bits & 1;
                bits >>= 1;
            }
        }

        // Alla pixlar i gruppen räknas om gruppen är stor nog
        if (count >= MATCH_SIZE) {
            for (uint8_t x = 0; x < boardwidth; x++) {
                matches[x] |= group[x];
            }
        }

        // Gruppen är färdigbehandlad
        for (uint8_t x = 0; x < boardwidth; x++) {
            remaining[x] &= ~group[x];
        }
    }

    // Inga matches
    bool found = false;

    for (uint8_t x = 0; x < boardwidth; x++) {
        if (matches[x]) {
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    // Bygg explosionsmask: match + alla 8 grannar
    uint8_t blast[boardwidth] = {};

    for (uint8_t x = 0; x < boardwidth; x++) {
        uint8_t vertical =
            matches[x]
            | (matches[x] << 1)
            | (matches[x] >> 1);

        blast[x] |= vertical;

        if (x > 0) {
            blast[x - 1] |= vertical;
        }

        if (x + 1 < boardwidth) {
            blast[x + 1] |= vertical;
        }
    }

    // Släck explosionen i både framebuffer och blinkmask
    for (uint8_t x = 0; x < boardwidth; x++) {
        console.insertColumn(x + boardOffset, getColumn(x + boardOffset) & ~blast[x]);
        console.state[x + boardOffset] = console.state[x + boardOffset] & ~blast[x];
    }

    return true;
}

bool ShiftApp::collapseDown() {
    bool changed = false;

    for (uint8_t x = 0; x < boardwidth; x++) {
        uint8_t writeY = 0;

        for (uint8_t y = 0; y < 8; y++) {
            bool screenBit = console.getPixel(x + boardOffset, y);
            bool ramBit = getRamBit(x + boardOffset, y);

            if (!screenBit && !ramBit) {
                continue;
            }

            if (y != writeY) {
                console.setPixel(x + boardOffset, writeY, screenBit);
                setRamBit(x + boardOffset, writeY, ramBit);

                console.setPixel(x + boardOffset, y, false);
                setRamBit(x + boardOffset, y, false);

                changed = true;
            }

            writeY++;
        }
    }

    return changed;
}

bool ShiftApp::collapseLeft() {
    bool changed = false;
    uint8_t writeX = 0;

    for (uint8_t x = 0; x < boardwidth; x++) {
        if (columnIsEmpty(x + boardOffset)) {
            continue;
        }

        if (x != writeX) {
            moveColumn(x + boardOffset, writeX + boardOffset);
            clearColumn(x + boardOffset);
            changed = true;
        }

        writeX++;
    }

    return changed;
}

bool ShiftApp::columnIsEmpty(uint8_t x) {
    return getColumn(x) == 0 && console.state[x] == 0;
}

void ShiftApp::moveColumn(uint8_t from, uint8_t to) {
    uint8_t screen = getColumn(from);
    uint8_t ram = console.state[from];

    console.insertColumn(to, screen);
    console.state[to] = ram;
}

void ShiftApp::clearColumn(uint8_t x) {
    console.insertColumn(x, 0);
    console.state[x] = 0;
}

void ShiftApp::begin() {
    console.clearScreen();
    createBoard();
}

void ShiftApp::update() {
    console.update();

    performBlink();

    uint16_t interval = (getState() == B0) ? 200 : 1000;

    if (!console.tickDue(interval)) return;
    
    uint8_t buttons = console.consumeButtons();

    if (getState() == B0) {
        if (buttons & BTN_UP){
             moveMarker(true);
        } else if (buttons & BTN_DOWN){
             moveMarker(false);
        } else if (buttons & BTN_ACTION){
            shiftRow();
        }
    } else {
        if (matchPixels() || collapseDown() || collapseLeft()) {
            setState(B0);
        }
    }

}
