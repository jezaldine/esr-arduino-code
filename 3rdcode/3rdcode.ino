#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


#define dryingLinearActuator1 D0
#define dryingLinearActuator2 D1
#define pushLinearActuator1 D2
#define pushLinearActuator2 D3
#define openLinearActuator1 D4
#define openLinearActuator2 D5
#define pulvorizer D6
#define pushButton D7

bool lastButtonState = LOW;
bool currentButtonState;

#define WIFI_SSID "betlog"
#define WIFI_PASSWORD "dandineee"

#define API_KEY "AIzaSyAuCoNK3FuFsHC_T1JQYTEK8Xh35AQMgUU"
#define DATABASE_URL "e-sugar-rush-5329b-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

unsigned long sendDataPrevMillis = 0;

void setup() {
  pinMode(pushButton, INPUT_PULLUP);
  Serial.begin(9600);
  pinMode(dryingLinearActuator1, OUTPUT);
  pinMode(dryingLinearActuator2, OUTPUT);
  pinMode(pushLinearActuator1, OUTPUT);
  pinMode(pushLinearActuator2, OUTPUT);
  pinMode(openLinearActuator1, OUTPUT);
  pinMode(openLinearActuator2, OUTPUT);
  pinMode(pulvorizer, OUTPUT);
  digitalWrite(dryingLinearActuator1, HIGH);
  digitalWrite(dryingLinearActuator2, HIGH);
  digitalWrite(pushLinearActuator1, HIGH);
  digitalWrite(pushLinearActuator2, HIGH);
  digitalWrite(openLinearActuator1, HIGH);
  digitalWrite(openLinearActuator2, HIGH);
  digitalWrite(pulvorizer, HIGH);
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


void emergencyButton() {
  currentButtonState = digitalRead(pushButton);

  if (currentButtonState == LOW && lastButtonState == HIGH) {
    digitalWrite(dryingLinearActuator1, HIGH);
    digitalWrite(dryingLinearActuator2, HIGH);
    digitalWrite(pushLinearActuator1, HIGH);
    digitalWrite(pushLinearActuator2, HIGH);
    digitalWrite(openLinearActuator1, HIGH);
    digitalWrite(openLinearActuator2, HIGH);
    digitalWrite(pulvorizer, HIGH);
    Firebase.RTDB.setBool(&fbdo, "Emergency/button", true);
    Serial.println("push button on");
  }

  lastButtonState = currentButtonState;
}


void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    int boilSizeValue;
    long startTime;
    startTime = millis();

    if (Firebase.RTDB.getInt(&fbdo, "Sizes/boilSize")) {
          int boilSize = fbdo.intData();
          boilSizeValue = boilSize;
          Serial.println(boilSizeValue);
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }

     if (Firebase.RTDB.getBool(&fbdo, "Controls/dry")) {
      if (fbdo.dataType() == "boolean"){
        bool dryStateStr = fbdo.boolData();
        Serial.println("Seccess: " + fbdo.dataPath() + ": " + dryStateStr + "(" + fbdo.dataType() + ")");
        if (dryStateStr == true) {
        Serial.println("Drying is working...");
        while (millis() - startTime < 1800000) {
            digitalWrite(dryingLinearActuator1, LOW);
            digitalWrite(dryingLinearActuator2, HIGH);
            delay(28000);  
            digitalWrite(dryingLinearActuator1, HIGH);
            digitalWrite(dryingLinearActuator2, LOW);
            delay(35000);  
        }
        digitalWrite(dryingLinearActuator1, HIGH);
        digitalWrite(dryingLinearActuator2, LOW);
        delay(35000);
        digitalWrite(dryingLinearActuator1, HIGH);
        digitalWrite(dryingLinearActuator2, HIGH);
  }
    }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

     if (Firebase.RTDB.getBool(&fbdo, "Controls/pulverize")) {
      if (fbdo.dataType() == "boolean"){
        bool pulverizeStateStr = fbdo.boolData();
        Serial.println("Seccess: " + fbdo.dataPath() + ": " + pulverizeStateStr + "(" + fbdo.dataType() + ")");
        bool pulverizeState = (pulverizeStateStr == false) ? HIGH : LOW;
        digitalWrite(pulvorizer, pulverizeState);
        if (pulverizeStateStr == true) {
          digitalWrite(openLinearActuator1, LOW);
          digitalWrite(openLinearActuator2, HIGH);
        }else {
          digitalWrite(openLinearActuator1, HIGH);
          digitalWrite(openLinearActuator2, LOW);
        }
       
    }
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

     if (Firebase.RTDB.getBool(&fbdo, "Controls/push")) {
      if (fbdo.dataType() == "boolean"){
        bool pushStateStr = fbdo.boolData();
        Serial.println("Seccess: " + fbdo.dataPath() + ": " + pushStateStr + "(" + fbdo.dataType() + ")");
        bool pushState = (pushStateStr == false) ? HIGH : LOW;
        if (pushStateStr == true){
          digitalWrite(pushLinearActuator1, LOW);
          digitalWrite(pushLinearActuator2, HIGH);
          delay(28000);
          digitalWrite(pushLinearActuator1, HIGH);
          digitalWrite(pushLinearActuator2, LOW);
          delay(35000);
          digitalWrite(pushLinearActuator1, HIGH);
          digitalWrite(pushLinearActuator2, HIGH);
        }else{
          digitalWrite(pushLinearActuator1, HIGH);
          digitalWrite(pushLinearActuator2, LOW);
        }
    }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

    emergencyButton();

    Serial.println("_______________________________________");
  }
}




