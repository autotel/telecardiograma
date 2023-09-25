#define PIN_MOTOR 9
#define PIN_PULSE_SENSOR A0
#define samplingRate 100

float originalValue = 0;
float lowpassed = 0;
float _lowpassed2 = 0;
float highpassed = 0;
float lowpassRatio = 0.1;
float highPassRatio = 0.1;
int interval = 1000 / samplingRate;
unsigned long nextMeasurement = 0;
int motorSpeed = 0;

struct PulseAnalyzer {
  // umbral de pulso que se auto-ajusta
  float movingThreshold = 2.;
  // mínimo del umbral, para prevenir detectar ruido
  float minThreshold = 1.6;
  float maxThreshold = 4.;
  bool prevSlopePositive = false;
  unsigned long lastPositiveCross = 0;
  float measuredFrequencyHertz = 0.;
  void thresholdCrossedUp(unsigned long now) {
    unsigned long deltaTimeMillis = now - lastPositiveCross;
    measuredFrequencyHertz = 1000. / deltaTimeMillis;
  }
  void inSample(float sampleValue, unsigned long now) {
    bool currentSlopePositive = sampleValue > movingThreshold;
    if (currentSlopePositive) {
      movingThreshold = min(sampleValue, maxThreshold);
      if (!prevSlopePositive) {
        thresholdCrossedUp(now);
      }
    } else {
      motorSpeed = 0;
      movingThreshold -= 40. / samplingRate;
      if (movingThreshold < minThreshold) {
        movingThreshold = minThreshold;
      }
    }
    prevSlopePositive = currentSlopePositive;
  }
} PulseAnalyzer;

void measure() {
  originalValue = analogRead(PIN_PULSE_SENSOR);
  lowpassed = lowpassRatio * originalValue + (1 - lowpassRatio) * lowpassed;
  _lowpassed2 = highPassRatio * lowpassed + (1 - highPassRatio) * _lowpassed2;
  highpassed = lowpassed - _lowpassed2;
}

void setup() {
  pinMode(PIN_PULSE_SENSOR, INPUT);
  pinMode(PIN_MOTOR, OUTPUT);
  analogWrite(PIN_MOTOR, motorSpeed);  // Configura la velocidad inicial del motor
  Serial.begin(9600);
}

void loop() {
  unsigned long now = millis();
  if (now > nextMeasurement) {
    nextMeasurement = now + interval;
    measure();
    Serial.print(-1);
    Serial.print(',');
    Serial.print(highpassed, 4);
    Serial.print(',');
    Serial.print(PulseAnalyzer.movingThreshold, 4);
    Serial.print(',');
    Serial.print(3);
    Serial.println();
    // analogWrite(motorPin, 0);
    PulseAnalyzer.inSample(highpassed, now);

    analogWrite(PIN_MOTOR, motorSpeed);  // Controla la velocidad del motor
  }
}