#define PIN_PULSE_SENSOR A0
#define samplingRate 100

#define MONITOR_SIGNALS
#define MONITOR_RATE
// #define DUMMY_HEARBEAT


// struct encargada de medir la frecuencia en la señal
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
    // funcion para llamar cuando el pulso cruza el umbral en dirección positiva
    // se mide el intervalo de tiempo respecto de la última vez, y así obtenemos
    // la frecuencia
    unsigned long deltaTimeMillis = now - lastPositiveCross;
    float hz = 1000. / (float)deltaTimeMillis;
    // promediar
    measuredFrequencyHertz = hz * 0.3 + measuredFrequencyHertz * 0.7;
    lastPositiveCross = now;
  }
  void inSample(float sampleValue, unsigned long now) {
    bool currentSlopePositive = sampleValue > movingThreshold;
    // crea una línea de comparación para reconocer un pulso, aunque este varíe
    if (currentSlopePositive) {
      movingThreshold = movingThreshold * 0.7 + sampleValue * 0.3;
      if (movingThreshold > maxThreshold) {
        movingThreshold = maxThreshold;
      };
      if (!prevSlopePositive) {
        thresholdCrossedUp(now);
      }
    } else {
      movingThreshold -= 2. / samplingRate;
      if (movingThreshold < minThreshold) {
        movingThreshold = minThreshold;
      }
    }
    prevSlopePositive = currentSlopePositive;
  }
} PulseAnalyzer;

// struct encargado de filtrar la señal
struct SignalFilter {

  float originalValue = 0;
  float lowpassed = 0;
  float _lowpassed2 = 0;
  float outputValue = 0;
  float lowpassRatio = 0.1;
  float highPassRatio = 0.1;

  int interval = 1000 / samplingRate;
  unsigned long nextMeasurement = 0;

  void measure() {
    originalValue = analogRead(PIN_PULSE_SENSOR);
    lowpassed = lowpassRatio * originalValue + (1 - lowpassRatio) * lowpassed;
    _lowpassed2 = highPassRatio * lowpassed + (1 - highPassRatio) * _lowpassed2;
    outputValue = lowpassed - _lowpassed2;
  }

  // llamar ésta en cada loop
  void loop() {
    unsigned long now = millis();
    if (now > nextMeasurement) {
      nextMeasurement = now + interval;
      measure();
    }
  }
} SignalFilter;


void setup() {
  pinMode(PIN_PULSE_SENSOR, INPUT);
#if defined(MONITOR_SIGNALS) || defined(MONITOR_RATE)
  Serial.begin(9600);
#endif
}

void loop() {
  unsigned long now = millis();

  SignalFilter.loop();


  float heartSignal = SignalFilter.outputValue;
#if defined(DUMMY_HEARBEAT)
  float r = PI * float(now) / 1000.;
  float s = sin(r);
  heartSignal = 2. * s * s;
#endif


  PulseAnalyzer.inSample(heartSignal, now);

#if defined(MONITOR_SIGNALS)
  Serial.print(-1);
  Serial.print(',');
  Serial.print(heartSignal, 4);
  Serial.print(',');
  Serial.print(PulseAnalyzer.movingThreshold, 4);
  Serial.print(',');
  Serial.print(3);
  Serial.print(',');
#endif
#if defined(MONITOR_RATE)
  Serial.print(PulseAnalyzer.measuredFrequencyHertz, 4);
  Serial.print(',');
#endif
#if defined(MONITOR_SIGNALS) || defined(MONITOR_RATE)
  Serial.println();
#endif
}