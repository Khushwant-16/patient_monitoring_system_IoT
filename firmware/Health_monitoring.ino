#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

// --- UPDATE THESE WITH YOUR DETAILS ---
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const String serverName = "http://YOUR_IPV4_ADDRESS:5000/sensor_data";

// --- SENSOR OBJECTS & PINS ---
MAX30105 particleSensor;
const int GSR_PIN = 34; // GSR Analog Input

// --- MAX30102 VARIABLES ---
uint32_t irBuffer[100]; 
uint32_t redBuffer[100];  
int32_t bufferLength = 100; 
int32_t spo2; 
int8_t validSPO2; 
int32_t heartRate; 
int8_t validHeartRate; 

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // 1. Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to Wi-Fi...");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");

  // 2. Initialize MAX30102 on default I2C (Pins 21 & 22)
  Serial.println("Initializing MAX30102 on Pins 21 & 22...");
  Wire.begin(21, 22);
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found! Check pins 21/22.");
    while (1);
  }
  
  // We set sample average to 16 to help stabilize your HR readings!
  particleSensor.setup(60, 16, 2, 100, 411, 4096); 
  Serial.println("MAX30102 Ready!");
  
  // 3. GSR Setup
  pinMode(GSR_PIN, INPUT);
  Serial.println("GSR Sensor Ready on Pin 34!");

  Serial.println("\n>>> ALL SENSORS READY! Place your finger on the MAX30102. <<<");
}

void loop() {
  // 4. Gather 100 samples from MAX30102 (Takes ~4 seconds)
  for (byte i = 0 ; i < bufferLength ; i++) {
    while (particleSensor.available() == false) 
      particleSensor.check(); 
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); 
  }

  // Calculate Heart Rate and SpO2
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  // If the algorithm successfully detects a pulse...
  if (validHeartRate == 1 && validSPO2 == 1 && heartRate < 200 && spo2 > 50) {
      
      // 5. Read and Smooth the GSR Data
      long gsrTotal = 0;
      for(int i = 0; i < 10; i++) {
        gsrTotal += analogRead(GSR_PIN);
        delay(5);
      }
      int smoothedGSR = gsrTotal / 10;

      // 6. Mock the missing hardware data
      float mocked_temp = 36.5;
      float mocked_motion = 0.80;

      // 7. Send the HTTP Request
      if(WiFi.status() == WL_CONNECTED){
        HTTPClient http;
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");
        
        // Construct JSON Payload. 
        // NOTE: We added "gsr" to the JSON. Make sure your Python app accepts this key!
        String jsonPayload = "{\"heart_rate\":" + String(heartRate) + 
                             ",\"spo2\":" + String(spo2) + 
                             ",\"temperature\":" + String(mocked_temp) + 
                             ",\"motion_magnitude\":" + String(mocked_motion) + 
                             ",\"gsr\":" + String(smoothedGSR) + "}";
                             
        Serial.println("\n>>> SENDING TO FUSION ENGINE: " + jsonPayload);
        int httpResponseCode = http.POST(jsonPayload);
        
        if (httpResponseCode > 0) {
          Serial.println(">>> SERVER DECISION: " + http.getString());
        } else {
          Serial.println(">>> SERVER ERROR: " + String(httpResponseCode));
        }
        http.end();
      }
  } else {
      Serial.println("Reading vitals... Keep finger perfectly still.");
  }
}