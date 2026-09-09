// Emit AVR witness replay: same columns and actions, actual SrbApp physics.
const fs = require('fs');
const path = require('path');
const {step} = require('./check.cjs');
const out = path.resolve(__dirname,'../../.pio/srb-experiment');
const cases = JSON.parse(fs.readFileSync(path.join(out,'witnesses.json')));
let source = `#include "apps/SrbApp.h"
unsigned long millis() { return 0; }
int analogRead(uint8_t) { return 1023; }
void delay(unsigned long) {}
static TinyConsoleGameApi console;
static SrbApp game(console);
volatile uint16_t failureLine, checks;
volatile uint8_t caseIndex;
volatile bool completed;
extern "C" __attribute__((noinline)) void testFinished() { asm volatile("nop"); }
#define CHECK(c) do { ++checks; if (!(c)) { failureLine=__LINE__; testFinished(); for (;;) {} } } while(0)
`;
cases.forEach((c,i) => {
    let s = [4,0,0,0], checksum = 0;
    // Last move triggers LEVEL_END before updating x in actual game.
    for (const a of c.actions.slice(0,-1)) {
        s = step(c.raw.map(x=>x&252), s, a&1, a&2 ? 1:0);
        for (const n of s.slice(0,3)) checksum = (checksum*31 + (n&255)) & 65535;
    }
    c.checksum = checksum;
    source += `const uint8_t columns${i}[] PROGMEM = {${c.raw}};\n`;
    source += `const uint8_t actions${i}[] PROGMEM = {${c.actions}};\n`;
});
source += `void run(uint8_t level, uint8_t s1, uint8_t s2, uint8_t s3,
         const uint8_t* cols, const uint8_t* actions, uint16_t count, uint16_t expected) {
    console.clearState(); console.clearScreen();
    console.state[1]=4; console.state[7]=level;
    console.state[4]=s1; console.state[5]=s2; console.state[6]=s3;
    for (uint16_t x=0; x<256; ++x) CHECK(game.getLevelColumn(x)==pgm_read_byte(cols+x));
    for (int16_t x=255; x>=0; --x) CHECK(game.getLevelColumn(x)==pgm_read_byte(cols+x));
    for (uint8_t x=0; x<16; ++x) console.insertColumn(x, game.getLevelColumn(x)&0xfc);
    game.drawLives(3);
    uint16_t checksum=0;
    for (uint16_t t=0; t<count; ++t) {
        uint8_t action=pgm_read_byte(actions+t);
        if (action&1) game.doJump();
        game.doGravity();
        bool end=false;
        if ((t&3)==0 && (action&2)) end=game.move(true);
        if (!end) CHECK(game.getLives()==3);
        if (t==count-1) { CHECK(end); CHECK(checksum==expected); }
        else {
            CHECK(!end);
            for (uint8_t n=1; n<=3; ++n) checksum=checksum*31+console.state[n];
        }
        // Terrain-only experiment: remove metadata-selected enemies before
        // the next tick. They spawn off to the right and are never drawn here.
        for (uint8_t n=12; n<16; ++n) console.state[n]=0;
    }
}
int main() {
`;
cases.forEach((c,i)=>source+=`caseIndex=${i}; run(${c.level},${c.seeds},columns${i},actions${i},sizeof(actions${i}),${c.checksum});\n`);
source+='completed=true; testFinished(); for (;;) {}\n}\n';
fs.writeFileSync(path.join(out,'replay.cpp'),source);
console.log('AVR replay source generated');
