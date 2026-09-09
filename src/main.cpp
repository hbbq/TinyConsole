#include <Arduino.h>
#include "TinyRandom.h"
#include "hardware/TinyConsole.h"
#include "apps/GameIcons.h"
#include "appConf.h"

TinyConsole console;
Game activeGame(console, GameId::Racer);

namespace {
constexpr uint8_t iconY = 0; // Icons occupy rows 0..5; row 6 stays blank.
constexpr uint8_t markerY = 7;
constexpr uint8_t scrollMs = 60;
constexpr uint8_t releaseDebounceMs = 30;
enum class LauncherPhase : uint8_t {
  Ready, ScrollLeft, ScrollRight, Release, LaunchHold, LaunchRelease, Playing
};
uint8_t selected = gameCount - 1;
uint8_t scrollStep = 0;
LauncherPhase phase = LauncherPhase::Ready;
uint16_t seed = 0;

uint8_t previousGame(uint8_t game) {
  return game == 0 ? gameCount - 1 : game - 1;
}
uint8_t nextGame(uint8_t game) {
  return game + 1 == gameCount ? 0 : game + 1;
}
void drawIcon(uint8_t game, uint8_t x) {
  for(uint8_t column = 0; column < 4; column++) {
    console.insertColumn(x + column,
        gameIconColumn(static_cast<GameId>(game), column) << iconY);
  }
}
void drawMarker() {
  // Repair the shifted old marker as well as drawing its fixed position.
  for(uint8_t x = 0; x < console.Width; x++) {
    console.setPixel(x, markerY, x == 7 || x == 8);
  }
}
void drawLauncher() {
  console.clearScreen();
  drawIcon(previousGame(selected), 1);
  drawIcon(selected, 6);
  drawIcon(nextGame(selected), 11);
  drawMarker();
}
void updateLauncher() {
  seed++;
  uint8_t buttons = console.readButtons();
  if(phase == LauncherPhase::LaunchHold) {
    if(console.tickDue(1000)) phase = LauncherPhase::LaunchRelease;
    return;
  }
  if(phase == LauncherPhase::Release || phase == LauncherPhase::LaunchRelease) {
    if(buttons) {
      console.tickDue(0); // Require 30 ms continuously released, as before.
    } else if(console.tickDue(releaseDebounceMs)) {
      if(phase == LauncherPhase::LaunchRelease) {
        tinyRandomSeed(seed);
        activeGame = GameFactory::get(console, static_cast<GameId>(selected));
        console.clearScreen();
        activeGame.begin();
        phase = LauncherPhase::Playing;
      } else {
        phase = LauncherPhase::Ready;
      }
    }
    return;
  }
  if(phase == LauncherPhase::ScrollLeft || phase == LauncherPhase::ScrollRight) {
    if(!console.tickDue(scrollMs)) return;
    bool left = phase == LauncherPhase::ScrollLeft;
    if(left) console.shiftLeft();
    else console.shiftRight();
    // Feed the four columns of the offscreen icon, then the outer margin.
    uint8_t incoming = left ? nextGame(nextGame(selected))
                            : previousGame(previousGame(selected));
    uint8_t column = scrollStep < 4
        ? gameIconColumn(static_cast<GameId>(incoming),
                         left ? scrollStep : 3 - scrollStep) << iconY
        : 0;
    console.insertColumn(left ? console.Width - 1 : 0, column);
    drawMarker();
    if(++scrollStep == 5) {
      selected = left ? nextGame(selected) : previousGame(selected);
      phase = LauncherPhase::Release;
    }
    return; // Runtime presents every individual pixel shift.
  }
  if(buttons & BTN_ACTION) {
    console.clearScreen();
    drawIcon(selected, 6);
    phase = LauncherPhase::LaunchHold;
    console.tickDue(0);
  } else if(buttons & (BTN_UP | BTN_DOWN)) {
    phase = buttons & BTN_UP ? LauncherPhase::ScrollRight : LauncherPhase::ScrollLeft;
    scrollStep = 0;
    console.tickDue(0);
  }
}
}

void setup() {
  console.begin();
  console.setBrightness(0);
  drawLauncher();
  console.updateDisplay();
}
void loop() {
  if(phase == LauncherPhase::Playing) activeGame.update();
  else updateLauncher();
  console.updateDisplay();
}
