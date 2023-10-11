namespace Temperatura {
unsigned long nextMeasurement = 0;
unsigned long interval = 5000;
float temperatureC = 0;
void setup() {
  pinMode(PIN_TEMPERATURE_SENSOR, INPUT);
}
void loop() {
  unsigned long now = millis();
  if (now > nextMeasurement) {
    nextMeasurement = now + interval;
    int sensorValue = analogRead(PIN_TEMPERATURE_SENSOR);
    float voltage = (sensorValue / 1024.0) * 5000.0;  // Convierte la lectura a voltaje en mV
    temperatureC = (voltage - 500.0) / 10.0;          // Convierte el voltaje a temperatura en grados Celsius
  }
}

}