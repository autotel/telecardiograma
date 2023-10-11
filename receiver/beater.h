#define MONITOR_BEATER

namespace Beater {

float beatLerp(float lerp) {
  if (lerp > 0.9) {
    return 0;
  } else if (lerp > 0.7) {
    return 0.5;
  } else if (lerp > 0.5) {
    return 0;
  } else if (lerp > 0.3) {
    return 1;
  } else {
    return 0;
  }
}
int bpm = 0;
float beatPhase = 0;
unsigned long prev = 0;

void setup() {
  pinMode(PIN_MOTOR, OUTPUT);
  #ifdef MONITOR_BEATER
  Serial.begin(115200);
  #endif
}

void loop() {
  float bps = float(bpm) / 60.;
  unsigned long now = millis();
  float delta = now - prev;
  beatPhase = fmod(beatPhase + bps * delta / 1000., 1.);

  float out = beatLerp(beatPhase);
  int mapped = floor(out * 255.);
  #ifdef MONITOR_BEATER
  Serial.print("1,0,");
  Serial.print(beatPhase);
  Serial.print(",");
  Serial.print(out);
  Serial.println();
  analogWrite(PIN_MOTOR, mapped);
  #endif
  prev = now;
}
}