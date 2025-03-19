#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define ONE_WIRE_BUS D0
#define echoPin D1
#define trigPin D2
#define ECHO_PIN D3
#define TRIG_PIN D4
#define MAX_DISTANCE_CM 30  
#define MAX_DISTANCE_CM_B 50  
#define MIN_DISTANCE_CM 4 
#define MIN_DISTANCE_CM_B 4 

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

#define WIFI_SSID "betlog"
#define WIFI_PASSWORD "dandineee"

#define API_KEY "AIzaSyAuCoNK3FuFsHC_T1JQYTEK8Xh35AQMgUU"
#define DATABASE_URL "e-sugar-rush-5329b-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;
long jsduration;
int jsdistance; 
long duration;
int distance; 

unsigned long sendDataPrevMillis = 0;


void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(9600);
  sensors.begin();
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

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase connection successful");
    signupOK = true;
  } else {
    Serial.printf("Firebase sign-up error: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

}

float getJuiceStorageDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2)                                                                                                     ;
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long jsduration = pulseIn(echoPin, HIGH);
  float jsdistance = jsduration * 0.0343 / 2;  // Convert to cm

  return jsdistance;
}

float getMainStorageDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2)                                                                                                     ;
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.0343 / 2;  // Convert to cm

  return distance;
}


int mapJuiceStorageDistanceToLevel(float juiceStorageDistance) {
  if (juiceStorageDistance < MIN_DISTANCE_CM) juiceStorageDistance = MIN_DISTANCE_CM;
  if (juiceStorageDistance > MAX_DISTANCE_CM) juiceStorageDistance = MAX_DISTANCE_CM;

  int jslevel = map(juiceStorageDistance, MAX_DISTANCE_CM, MIN_DISTANCE_CM, 0, 5);
  return jslevel;
}

int mapMainStorageDistanceToLevel(float mainStorageDistance) {
  if (mainStorageDistance < MIN_DISTANCE_CM_B) mainStorageDistance = MIN_DISTANCE_CM_B;
  if (mainStorageDistance > MAX_DISTANCE_CM_B) mainStorageDistance = MAX_DISTANCE_CM_B;

  int level = map(mainStorageDistance, MAX_DISTANCE_CM_B, MIN_DISTANCE_CM_B, 0, 15);
  return level;
}

void loop() {

   if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
     sensors.requestTemperatures(); 
      float temp = sensors.getTempCByIndex(0);
      float mainStorageDistance = getMainStorageDistance();
      int mainStorageLevel = mapMainStorageDistanceToLevel(mainStorageDistance);
      float juiceStorageDistance = getJuiceStorageDistance();
      int juiceStorageLevel = mapJuiceStorageDistanceToLevel(juiceStorageDistance);
      Firebase.RTDB.setFloat(&fbdo, "Sensors/temperature", temp);
      Firebase.RTDB.setInt(&fbdo, "Sensors/juiceStorage", juiceStorageLevel);
      Firebase.RTDB.setInt(&fbdo, "Sensors/mainStorage", mainStorageLevel);
      Serial.print("Celsius temperature: ");
      Serial.print(temp); 

      Serial.print("JuiceStorageDistance: ");
      Serial.print(juiceStorageDistance);
      Serial.print(" cm, Water Level: ");
      Serial.println(juiceStorageLevel);

      Serial.print("MainStorageDistance: ");
      Serial.print(mainStorageDistance);
      Serial.print(" cm, Water Level: ");
      Serial.println(mainStorageLevel);

    }

}

