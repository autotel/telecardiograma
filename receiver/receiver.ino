#define PIN_MOTOR A1
#include "beater.h"

void setup() {
  Beater::setup();
}

void loop() {
  Beater::bpm = 62;
  Beater::loop();
}