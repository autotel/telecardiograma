#define PIN_MOTOR 9
#define PIN_PULSE_SENSOR A0


double originalValue = 0;
double lowpassed = 0;
double _lowpassed2 = 0;
double highpassed = 0;
float lowpassRatio = 0.05;
float highPassRatio = 0.01;
int samplingRate = 100;
int interval = 1000 / samplingRate;
unsigned long nextMeasurement = 0;
int motorSpeed = 0;

// umbral de pulso que se auto-ajusta
double movingThreshold = 2;
// mínimo del umbral, para prevenir detectar ruido
double minThreshold = 2;

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
    Serial.print(highpassed);
    Serial.print(',');
    Serial.print(movingThreshold);
    Serial.println();
    // analogWrite(motorPin, 0);

    if (highpassed > movingThreshold) {
      motorSpeed = 255;  // Puedes ajustar la velocidad según tus preferencias
      movingThreshold = highpassed;
    } else {
      motorSpeed = 0;
      movingThreshold *= samplingRate / 1000.00;
    }

    analogWrite(PIN_MOTOR, motorSpeed); // Controla la velocidad del motor
  }
}