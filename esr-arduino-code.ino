#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define powerPin D0
#define extractPin D1
#define boilPin1 D2
#define boilPin2 D3 
#define rotateBoiler D4
#define pumpToMainStorage D5
#define pumpToBoilPin D6
#define pumpToJuicePin D7
#define transferingToDrying D8

#define WIFI_SSID "betlog"
#define WIFI_PASSWORD "dandineee"

#define API_KEY "AIzaSyAuCoNK3FuFsHC_T1JQYTEK8Xh35AQMgUU"
#define DATABASE_URL "e-sugar-rush-5329b-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;
int boilSize = 0;
int transferSize = 0;


unsigned long sendDataPrevMillis = 0;

void setup() {
  pinMode(powerPin, OUTPUT);
  pinMode(extractPin, OUTPUT);
  pinMode(boilPin1, OUTPUT);
  pinMode(boilPin2, OUTPUT);
  pinMode(pumpToBoilPin, OUTPUT);
  pinMode(pumpToMainStorage, OUTPUT);
  pinMode(pumpToJuicePin, OUTPUT);
  pinMode(rotateBoiler, OUTPUT);
  pinMode(transferingToDrying, OUTPUT);
  digitalWrite(powerPin, HIGH);
  digitalWrite(extractPin, HIGH);
  digitalWrite(boilPin1, HIGH);
  digitalWrite(boilPin2, HIGH);
  digitalWrite(pumpToBoilPin, HIGH);
  digitalWrite(pumpToMainStorage, HIGH);
  digitalWrite(pumpToJuicePin, HIGH);
  digitalWrite(rotateBoiler, HIGH);
  digitalWrite(transferingToDrying, HIGH);
  Serial.begin(9600);
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

void emergencyCallback(bool isEmergency) {
  if (isEmergency){
    digitalWrite(powerPin, HIGH);
    digitalWrite(extractPin, HIGH);
    digitalWrite(boilPin1, HIGH);
    digitalWrite(boilPin2, HIGH);
    digitalWrite(pumpToBoilPin, HIGH);
    digitalWrite(pumpToMainStorage, HIGH);
    digitalWrite(pumpToJuicePin, HIGH);
    digitalWrite(rotateBoiler, HIGH);
    digitalWrite(transferingToDrying, HIGH);
    Firebase.RTDB.setBool(&fbdo, "Controls/boil", false);
    Firebase.RTDB.setBool(&fbdo, "Controls/dry", false);
    Firebase.RTDB.setBool(&fbdo, "Controls/extract", false);
    Firebase.RTDB.setBool(&fbdo, "Controls/filtered", false);
    Firebase.RTDB.setBool(&fbdo, "Controls/power", false);
    Firebase.RTDB.setBool(&fbdo, "Controls/startExtraction", false);
    Firebase.RTDB.setBool(&fbdo, "Controls/startTransfering", false);
    Firebase.RTDB.setBool(&fbdo, "Pass/isCooking", false);
    Firebase.RTDB.setBool(&fbdo, "Pass/isDrying", false);
    Firebase.RTDB.setBool(&fbdo, "Pass/pulvorizer", false);
    Firebase.RTDB.setBool(&fbdo, "Pass/isDrying", false);
    Firebase.RTDB.setBool(&fbdo, "Pass/transferToDrying", false);
    Firebase.RTDB.setBool(&fbdo, "Pass/transferToPulvorizer", false);
    Firebase.RTDB.setBool(&fbdo, "Emergency/button", true);
    Firebase.RTDB.setBool(&fbdo, "Emergency/button", false);
  }
}


void pumpToBoiler(int boilSizeValue, bool isExtractionStart) {
  if (boilSizeValue != 0 && isExtractionStart){
      int transferingTime = 50000 * boilSizeValue;
      digitalWrite(pumpToBoilPin, LOW);
      digitalWrite(transferingToDrying, HIGH);
      Serial.println("Pump to Boiler is working...");
      Firebase.RTDB.setInt(&fbdo, "Timer/juiceToBoiler", transferingTime);
      delay(transferingTime);
      digitalWrite(pumpToBoilPin, HIGH);
      Firebase.RTDB.setBool(&fbdo, "Controls/startExtraction", false);
      // Serial.println("Pump to Boiler is STOP...");
      // Firebase.RTDB.setInt(&fbdo, "Timer/cooking", cookingTime);
      // Firebase.RTDB.setBool(&fbdo, "Pass/isCooking", true);
      // digitalWrite(rotateBoiler, LOW);
      // digitalWrite(boilPin1, LOW);
      // digitalWrite(boilPin2, LOW);
      // Serial.println("Boiler is working...");
      // delay(cookingTime);
      // Firebase.RTDB.setBool(&fbdo, "Pass/isCooking", false);
      // digitalWrite(rotateBoiler, HIGH);
      // digitalWrite(boilPin1, HIGH);
      // digitalWrite(boilPin2, HIGH);
      // Serial.println("Boiler is stop...");
      // Firebase.RTDB.setBool(&fbdo, "Pass/transferToDrying", true);
      // digitalWrite(transferingToDrying, LOW);
      // Serial.println("Transfering to Drying...");
      // delay(90000 * boilSizeValue);
      // Serial.println("Transfering to drying is stop...");
      // digitalWrite(transferingToDrying, HIGH);
      // Firebase.RTDB.setBool(&fbdo, "Pass/transferToDrying", false);
      // Firebase.RTDB.setBool(&fbdo, "Pass/isDrying", true);
    }
}

void pumpToJuiceStorage(int transferSizeValue, bool isTransferingStart) {
   if (transferSizeValue != 0 && isTransferingStart){
      digitalWrite(pumpToJuicePin, LOW);
      Serial.println("Pump to juice storage is working...");
      int time = 40000 * transferSizeValue;
      Firebase.RTDB.setInt(&fbdo, "Timer/juiceToJuiceStorage", time);
      delay(time);
      digitalWrite(pumpToJuicePin, HIGH);
      Firebase.RTDB.setBool(&fbdo, "Controls/startTransfering", false);
      Serial.println("Pump to Juice Storage is STOP...");
    }
}


void pumpToMainStorageMethod() {
  if (Firebase.RTDB.getBool(&fbdo, "Controls/filtered")) {
      if (fbdo.dataType() == "boolean"){
        bool pumpToMainStorageStr = fbdo.boolData();
        Serial.println("Seccess: " + fbdo.dataPath() + ": " + pumpToMainStorageStr + "(" + fbdo.dataType() + ")");
        bool value = (pumpToMainStorageStr == false) ? HIGH : LOW;
        digitalWrite(pumpToMainStorage, value);
    }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    int boilSizeValue;
    bool isExtractionStart;
    int transferSizeValue;
    bool isTransferingStart;
    bool isEmergency;

    if (Firebase.RTDB.getBool(&fbdo, "Controls/power")) {
      if (fbdo.dataType() == "boolean"){
      bool powerStateStr = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + powerStateStr + "(" + fbdo.dataType() + ")");
      bool powerState = (powerStateStr == false) ? HIGH : LOW;
      digitalWrite(powerPin, powerState);   
      }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getBool(&fbdo, "Controls/extract")) {
      if (fbdo.dataType() == "boolean"){
        bool extractStateStr = fbdo.boolData();
        Serial.println("Seccess: " + fbdo.dataPath() + ": " + extractStateStr + "(" + fbdo.dataType() + ")");
        bool extractState = (extractStateStr == false) ? HIGH : LOW;
        digitalWrite(extractPin, extractState);
    }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.getBool(&fbdo, "Controls/boil")) {
      if (fbdo.dataType() == "boolean"){
        bool boilStateStr = fbdo.boolData();
        Serial.println("Seccess: " + fbdo.dataPath() + ": " + boilStateStr + "(" + fbdo.dataType() + ")");
        bool boilState = (boilStateStr == false) ? HIGH : LOW;
        digitalWrite(rotateBoiler, boilState);
        digitalWrite(boilPin1, boilState);
        digitalWrite(boilPin2, boilState);
    }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

     if (Firebase.RTDB.getBool(&fbdo, "Controls/dry")) {
      if (fbdo.dataType() == "boolean"){
        bool dryStateStr = fbdo.boolData();
        Serial.println("Seccess: " + fbdo.dataPath() + ": " + dryStateStr + "(" + fbdo.dataType() + ")");
        Firebase.RTDB.setBool(&fbdo, "Pass/isDrying", dryStateStr);
        bool dryState = (dryStateStr == false) ? HIGH : LOW;
        digitalWrite(transferingToDrying, dryState);
    }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getInt(&fbdo, "Sizes/boilSize")) {
          boilSize = fbdo.intData();
          Serial.print("Seccess! Boil: ");
          Serial.println(boilSize);
          boilSizeValue = boilSize;
         
    } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.getInt(&fbdo, "Sizes/transferSize")) {
          transferSize = fbdo.intData();
          Serial.print("Seccess! Transfer: ");
          Serial.println(transferSize);
          transferSizeValue = transferSize;
         
    } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getBool(&fbdo, "Controls/startExtraction")) {
      if (fbdo.dataType() == "boolean"){
          bool startExtractionStateStr = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + startExtractionStateStr + "(" + fbdo.dataType() + ")");
          // bool startExtractionState = (startExtractionStateStr == false) ? HIGH : LOW;
          // digitalWrite(pumpToBoilPin, startExtractionState);
          isExtractionStart = startExtractionStateStr;
      }
    } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.getBool(&fbdo, "Controls/startTransfering")) {
      if (fbdo.dataType() == "boolean"){
          bool startTransferingStateStr = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + startTransferingStateStr + "(" + fbdo.dataType() + ")");
          bool startTransferingState = (startTransferingStateStr == false) ? HIGH : LOW;
          digitalWrite(pumpToJuicePin, startTransferingState);
          isTransferingStart = startTransferingStateStr;
      }
    } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }
    
    if (Firebase.RTDB.getBool(&fbdo, "Emergency/button")) {
          bool emergencyValue = fbdo.boolData();
          Serial.println(emergencyValue);
          isEmergency = emergencyValue;
    } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

      pumpToBoiler(boilSizeValue, isExtractionStart);
      pumpToJuiceStorage(transferSizeValue,isTransferingStart );
      pumpToMainStorageMethod();
      emergencyCallback(isEmergency);
 

    Serial.println("_______________________________________");
  }
}




