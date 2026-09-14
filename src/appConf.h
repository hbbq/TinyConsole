#pragma once

// Set a game to 0 to omit it from the firmware and launcher.
#define ENABLE_RACER 1
#define ENABLE_BREAKOUT 1
#define ENABLE_SKYHOP 1
#define ENABLE_SHIFT 1
#define ENABLE_SRB 1

#if !ENABLE_RACER && !ENABLE_BREAKOUT && !ENABLE_SKYHOP && !ENABLE_SHIFT && !ENABLE_SRB
#error "At least one game must be enabled"
#endif
