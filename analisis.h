#define samplingRate 100

// qué cosas mandar a Serial
#define MONITOR_SIGNALS
#define MONITOR_RATE
// de si usar una señal falsa para probar.
// #define DUMMY_HEARBEAT

namespace Analysis {

unsigned long nextMeasurement = 0;
int interval = 1000 / samplingRate;


// struct encargada de medir la frecuencia en la señal
struct PulseAnalyzer {
  // umbral de pulso que se auto-ajusta
  float movingThreshold = 2.;
  // mínimo del umbral, para prevenir detectar ruido
  float minThreshold = 1.6;
  float maxThreshold = 13.;
  bool prevSlopePositive = false;
  unsigned long lastPositiveCross = 0;
  unsigned long lastMeasurement = 0;
  float measuredFrequencyHertz = 0.;
  unsigned int stateshow = 0;
  void thresholdCrossedUp(unsigned long now, unsigned long interval) {
    // funcion para llamar cuando el pulso cruza el umbral en dirección positiva
    // se mide el intervalo de tiempo respecto de la última vez, y así obtenemos
    // la frecuencia
    unsigned long deltaTimeMillis = now - lastPositiveCross;
    float hz = 1000. / (float)deltaTimeMillis;
    measuredFrequencyHertz = hz;
    lastPositiveCross = now;
    stateshow = 3;
  }
  void thresholdNotCrossed(unsigned long now, unsigned long interval) {
    unsigned long deltaTimeMillis = now - lastPositiveCross;
    // si no se ha detectado pulso en mas de 1.2s, setear la frecuencia en cero
    if (deltaTimeMillis > 1200) {
      measuredFrequencyHertz = 0;
    }
    stateshow = 0;
  }
  void inSample(float sampleValue, unsigned long now) {
    bool currentSlopePositive = sampleValue > movingThreshold;
    unsigned long interval = now - lastMeasurement;
    // crea una línea de comparación para reconocer un pulso, aunque este varíe
    if (currentSlopePositive) {
      movingThreshold = movingThreshold * 0.9 + sampleValue * 0.1;
      if (movingThreshold > maxThreshold) {
        movingThreshold = maxThreshold;
      };
      if (!prevSlopePositive) {
        thresholdCrossedUp(now, interval);
      } else {
        thresholdNotCrossed(now, interval);
      }
    } else {
      movingThreshold *= 0.999;
      if (movingThreshold < minThreshold) {
        movingThreshold = minThreshold;
      }
      thresholdNotCrossed(now, interval);
    }
    prevSlopePositive = currentSlopePositive;
    lastMeasurement = now;
  }
} PulseAnalyzer;

// struct encargado de filtrar la señal
struct SignalFilter {

  float originalValue = 0;
  float lowpassed = 0;
  float _lowpassed2 = 0;
  float outputValue = 0;
  float lowpassRatio = 12 / (float)samplingRate;
  float highPassRatio = 5 / (float)samplingRate;
  void measure() {
    originalValue = analogRead(PIN_PULSE_SENSOR);
    lowpassed = lowpassRatio * originalValue + (1 - lowpassRatio) * lowpassed;
    _lowpassed2 = highPassRatio * lowpassed + (1 - highPassRatio) * _lowpassed2;
    outputValue = lowpassed - _lowpassed2;
  }

} SignalFilter;

void setup() {
  pinMode(PIN_PULSE_SENSOR, INPUT);
#if defined(MONITOR_SIGNALS) || defined(MONITOR_RATE)
  Serial.begin(115200);
#endif
}

void loop() {


  unsigned long now = millis();
  if (now > nextMeasurement) {
    nextMeasurement = now + interval;
    SignalFilter.measure();



    float heartSignal = SignalFilter.outputValue;
#if defined(DUMMY_HEARBEAT)
    float r = PI * float(now) / 1000.;
    float s = sin(r);
    heartSignal = 2. * s * s;
    /*
  if (now > 10000) {
    heartSignal = 0;
  }
  */
#endif


    PulseAnalyzer.inSample(heartSignal, now);


#if defined(MONITOR_SIGNALS)
    Serial.print(-3);
    Serial.print(',');
    Serial.print(heartSignal);
    Serial.print(',');
    Serial.print(PulseAnalyzer.movingThreshold);
    Serial.print(',');
    Serial.print(15);
    Serial.print(',');
    Serial.print(PulseAnalyzer.stateshow);
    Serial.print(',');
#endif
#if defined(MONITOR_RATE)
    Serial.print(PulseAnalyzer.measuredFrequencyHertz);
    Serial.print(',');
#endif
#if defined(MONITOR_SIGNALS) || defined(MONITOR_RATE)
    Serial.println();
#endif
  }
}

float getFrequency() {
  return PulseAnalyzer.measuredFrequencyHertz;
}



}
