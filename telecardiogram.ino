
// #define USE_FIREBASE
// #define LOG_FIREBASE

#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#include "pins.h"
#include "analisis.h"

/* 1. Define the WiFi credentials */
#define WIFI_SSID "TRUCHA_BASICO"
#define WIFI_PASSWORD "arcoiris"

/* 2. Define the API Key */
#define API_KEY " -- API KEY --"

/* 3. Define the RTDB URL */
#define DATABASE_URL " -- URL --"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "-- e mail --"
#define USER_PASSWORD "-- password --"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long nextSendDataTime = 10000;



void setup() {
  Analysis::setup();

#if defined(LOG_FIREBASE) && defined(USE_FIREBASE)
  Serial.begin(115200);
#endif

#if defined(USE_FIREBASE)
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
#endif
}

void loop() {
  unsigned long now = millis();
  Analysis::loop();

#if defined(USE_FIREBASE)
  if (Firebase.ready() && (now > nextSendDataTime)) {
    float f = Analysis::getFrequency();
    nextSendDataTime = now + 5000;
#if defined(LOG_FIREBASE)
    Serial.print("float value is");
    Serial.print(f);
    Serial.printf(" sending... %s\n", Firebase.setFloat(fbdo, F("/hertz"), f) ? "ok" : fbdo.errorReason().c_str());
    Serial.println();
#endif
  }
#endif
}