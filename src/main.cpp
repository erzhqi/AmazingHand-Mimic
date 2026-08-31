/*
    main.cpp
    Erik Zhu
    August 31, 2026
    Firmware to control the AmazingHand's servos through a FE-URT-2 bus. The code receives 
    comma separated bend percentages from CameraProcessing.py and writes them to each finger
    via SyncWritePos to match movements of my hand.

*/

#include <Arduino.h>
#include <SCServo.h> //Library for controlling Feetech STS & SMS series servos

SMS_STS handServo;

#define HAND_RX_PIN 13
#define HAND_TX_PIN 14

HardwareSerial handSerial(1); // Setting up serial for communication with motors via a FE-URT-2

// Two motors per finger
// uint8_t INDEX[2] = {1, 2};
// uint8_t MIDDLE[2] = {3, 4};
// uint8_t RING[2] = {5, 6};
// uint8_t THUMB[2] = {7, 8};

uint8_t ALL_FINGERS[8] = {1, 2, 3, 4, 5, 6, 7, 8};

uint16_t SPEED[8] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};
uint8_t ACCELERATION[8] = {50, 50, 50, 50, 50, 50, 50, 50};

String pythonText;
boolean motorsOn = false;
double bendPercentages[4];

void parseValues(String originalString, double bendValues[4]);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initializing serial for hand
  handSerial.begin(1000000, SERIAL_8N1, HAND_RX_PIN, HAND_TX_PIN);
  handServo.pSerial = &handSerial;
  delay(200);
}

void loop() {
  // Repeatedly reading from serial
  if (Serial.available() > 0){
    pythonText = Serial.readStringUntil('\n');
    pythonText.trim();
    if (pythonText == "Motors Active!"){ // Activate motors
      motorsOn = true;
      Serial.println("Motors Active");
    }
    else if (motorsOn){
      // After motors are activated, parse values sent from python script and write values to servo
      Serial.println(pythonText);
      parseValues(pythonText, bendPercentages); // Writing bend percentages to array
      int16_t HAND_BEND_POSITIONS[8] = {
        1875+round(1169*bendPercentages[0]), 2183-round(1145*bendPercentages[0]),
        1926+round(1258*bendPercentages[1]), 2169-round(1068*bendPercentages[1]),
        2040+round(1136*bendPercentages[2]), 2108-round(1273*bendPercentages[2]),
        1978+round(1213*bendPercentages[3]), 2370-round(1368*bendPercentages[3])
      };
      // Synchronously writing values to all motors
      handServo.SyncWritePosEx(ALL_FINGERS, 8, HAND_BEND_POSITIONS, SPEED, ACCELERATION);
      }
    }
}

// Parsing from a string in the format "num1,num2,num3,num4," into an array of doubles (eg. {0.67, 1, 0.06, 0.07})
void parseValues(String originalString, double bendValues[4]){
  for (int i = 0; i < 4; i ++){
    int indexComma = originalString.indexOf(',');
    bendValues[i] = (originalString.substring(0, indexComma)).toDouble(); // Take a substring up until a comma
    originalString = originalString.substring(indexComma+1); // Modifying the original string
  }
}
