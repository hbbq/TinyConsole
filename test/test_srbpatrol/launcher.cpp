#include "hardware/TinyConsole.h"
#include "apps/GameIcons.h"

extern TinyConsole console;
extern unsigned long now;
extern int buttonAdc;
void setup();
void loop();

static void tick(uint16_t elapsed, int adc = 1023) {
    now += elapsed;
    buttonAdc = adc;
    loop();
}

static bool selectedIcon(GameId id) {
    for (uint8_t x = 0; x < 4; x++) {
        for (uint8_t y = 0; y < 6; y++) {
            if (console.getPixel(6 + x, y) != bool(gameIconColumn(id, x) & (1 << y))) return false;
        }
    }
    return console.getPixel(7, 7) && console.getPixel(8, 7);
}

uint8_t checkLauncher() {
    buttonAdc = 1023;
    setup();
    if (!selectedIcon(GameId::Srb)) return __LINE__;
    tick(40, 710); // Navigate left to Shift, then release.
    for (uint8_t i = 0; i < 5; i++) tick(60);
    tick(30);
    if (!selectedIcon(GameId::Shift)) return __LINE__;
    tick(40, 850); // Navigate back to SRB.
    for (uint8_t i = 0; i < 5; i++) tick(60);
    tick(30);
    if (!selectedIcon(GameId::Srb)) return __LINE__;
    tick(40, 920); // Launch, wait for the hold and release phases.
    tick(1000);
    tick(30);
    if (console.state[0] != 0x80 || console.state[7] != 1) return __LINE__;
    tick(1000);
    if (console.state[0] != 0 || console.state[8] != 0) return __LINE__;
    if (!console.getPixel(4, 0) || !console.getPixel(4, 7)) return __LINE__;
    tick(40);
    return console.state[0] == 1 ? 0 : __LINE__;
}
