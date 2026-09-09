#include "wokwi-api.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct {
  pin_t up;
  pin_t down;
  pin_t action;
  pin_t out;
} chip_state_t;

static void refresh_output(chip_state_t *chip) {
  bool up     = pin_read(chip->up) == LOW;
  bool down   = pin_read(chip->down) == LOW;
  bool action = pin_read(chip->action) == LOW;

  float voltage;

  if (up && down && action) {
    voltage = 5.0f * 581.0f / 1023.0f;
  } else if (up && down) {
    voltage = 5.0f * 619.0f / 1023.0f;
  } else if (up && action) {
    voltage = 5.0f * 664.0f / 1023.0f;
  } else if (up) {
    voltage = 5.0f * 714.0f / 1023.0f;
  } else if (down && action) {
    voltage = 5.0f * 769.0f / 1023.0f;
  } else if (down) {
    voltage = 5.0f * 839.0f / 1023.0f;
  } else if (action) {
    voltage = 5.0f * 922.0f / 1023.0f;
  } else {
    voltage = 5.0f;
  }

  pin_dac_write(chip->out, voltage);
}

static void on_pin_change(void *user_data, pin_t pin, uint32_t value) {
  chip_state_t *chip = (chip_state_t *)user_data;
  refresh_output(chip);
}

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  chip->up     = pin_init("UP", INPUT_PULLUP);
  chip->down   = pin_init("DOWN", INPUT_PULLUP);
  chip->action = pin_init("ACTION", INPUT_PULLUP);
  chip->out    = pin_init("OUT", ANALOG);

  const pin_watch_config_t watch_config = {
    .edge = BOTH,
    .pin_change = on_pin_change,
    .user_data = chip
  };

  pin_watch(chip->up, &watch_config);
  pin_watch(chip->down, &watch_config);
  pin_watch(chip->action, &watch_config);

  refresh_output(chip);
}
