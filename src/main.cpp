#include <Arduino.h>
#include <SCServo.h>

// Two independent servo communication buses for arm and hand
SMS_STS handServo;
SMS_STS armServo;

#define HAND_RX_PIN 13
#define HAND_TX_PIN 14

#define ARM_RX_PIN 26
#define ARM_TX_PIN 27

// Two serial buses for hand and arm
HardwareSerial handSerial(1);
HardwareSerial armSerial(2);

// Setting constants for each motor
const uint8_t FINGER1_RID = 1;
const uint8_t FINGER1_LID = 2;
const uint8_t FINGER2_RID = 3;
const uint8_t FINGER2_LID = 4;
const uint8_t FINGER3_RID = 5;
const uint8_t FINGER3_LID = 6;
const uint8_t THUMB_LID = 7; // This is the one that is "reversed"
const uint8_t THUMB_RID = 8;

const uint8_t ARM1 = 1;
const uint8_t ARM2 = 2;
const uint8_t ARM3 = 3;
const uint8_t ARM4 = 4;
const uint8_t ARM5 = 5;

uint8_t hand_servo_list[] = {FINGER1_RID, FINGER1_LID, FINGER2_RID, FINGER2_LID, FINGER3_RID, FINGER3_LID, THUMB_LID, THUMB_RID};
const uint8_t NUM_HAND_SERVOS = sizeof(hand_servo_list);

uint8_t arm_servo_list[] = {ARM1, ARM2, ARM3, ARM4, ARM5};
const uint8_t NUM_ARM_SERVOS = sizeof(arm_servo_list);

uint8_t INDEX[2] = {1, 2};
uint8_t MIDDLE[2] = {3, 4};
uint8_t RING[2] = {5, 6};
uint8_t THUMB[2] = {7, 8};

uint8_t ALL_FINGERS[8] = {1, 2, 3, 4, 5, 6, 7, 8};

int16_t INDEX_POSITION_CLOSE[2] = {3044, 1038};
int16_t INDEX_POSITION_OPEN[2] = {1875, 2183};
  
int16_t MIDDLE_POSITION_CLOSE[2] = {3184, 1101};
int16_t MIDDLE_POSITION_OPEN[2] = {1926, 2169};

int16_t RING_POSITION_CLOSE[2] = {3176, 835};
int16_t RING_POSITION_OPEN[2] = {2040, 2108};

int16_t THUMB_POSITION_CLOSE[2] = {3191, 1002};
int16_t THUMB_POSITION_OPEN[2] = {1978, 2370};

uint16_t SPEED[2] = {1000, 1000};
uint8_t ACCELERATION[2] = {50, 50};

String pythonText;
boolean motorsOn = false;
double bendPercentages[4];

void ping_servos();
void finger_movement_test(int index);
void parseValues(String originalString, double bendValues[4]);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);

  // Initializing serial for hand
  handSerial.begin(1000000, SERIAL_8N1, HAND_RX_PIN, HAND_TX_PIN);
  handServo.pSerial = &handSerial;
  delay(200);

  // Initializing serial for arm
  armSerial.begin(1000000, SERIAL_8N1, ARM_RX_PIN, ARM_TX_PIN);
  armServo.pSerial = &armSerial;
  delay(200);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0){
    pythonText = Serial.readStringUntil('\n');
    pythonText.trim();
    if (pythonText == "Motors Active!"){
      motorsOn = true;
      Serial.println("Motors Active");
    }
    else if (motorsOn){
      Serial.println(pythonText);
      parseValues(pythonText, bendPercentages);
      int16_t HAND_BEND_POSITIONS[8] = {
        1875+round(1169*bendPercentages[0]), 2183-round(1145*bendPercentages[0]),
        1926+round(1258*bendPercentages[1]), 2169-round(1068*bendPercentages[1]),
        2040+round(1136*bendPercentages[2]), 2108-round(1273*bendPercentages[2]),
        1978+round(1213*bendPercentages[3]), 2370-round(1368*bendPercentages[3])
      };
      handServo.SyncWritePosEx(ALL_FINGERS, 8, HAND_BEND_POSITIONS, SPEED, ACCELERATION);
      }
    }
}

void parseValues(String originalString, double bendValues[4]){
  for (int i = 0; i < 4; i ++){
    int indexComma = originalString.indexOf(',');
    bendValues[i] = (originalString.substring(0, indexComma)).toDouble();
    originalString = originalString.substring(indexComma+1);
  }
}

void servo_check(){
  Serial.println("Checking HAND Servos:");
  for (int i = 0; i < NUM_HAND_SERVOS; i++){
    int pingResult = handServo.Ping(hand_servo_list[i]);
    if (pingResult != -1){
      Serial.println("Response from Servo " + String(hand_servo_list[i]));
      int finger_pos = handServo.ReadPos(hand_servo_list[i]);
      Serial.println("Finger Position: " + String(finger_pos));
      Serial.println("-------");
    }
    else{
      Serial.println("No Response from Servo " + String(hand_servo_list[i]));
    }
  }
  Serial.println("*******");
  Serial.println("Checking ARM Servos:");
  for (int i = 0; i < NUM_ARM_SERVOS; i++){
    int pingResult = armServo.Ping(arm_servo_list[i]);
    if (pingResult != -1){
      Serial.println("Response from Servo " + String(arm_servo_list[i]));
      int arm_pos = armServo.ReadPos(arm_servo_list[i]);
      Serial.println("Arm Motor Position: " + String(arm_pos));
      Serial.println("-------");
    }
    else{
      Serial.println("No Response from Servo " + String(arm_servo_list[i]));
    }
  }
}

void finger_movement_test(int index){  
  Serial.println("Beginning Positions:");
  if (index == 0){
    Serial.println(handServo.ReadPos(1));
    Serial.println(handServo.ReadPos(2));
    Serial.println("Beginning to move finger");
    handServo.SyncWritePosEx(INDEX, 2, INDEX_POSITION_OPEN, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("Finger finished moving");
    handServo.SyncWritePosEx(INDEX, 2, INDEX_POSITION_CLOSE, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("Ending Positions:");
    Serial.println(handServo.ReadPos(1));
    Serial.println(handServo.ReadPos(2));
  }
  else if (index == 1){
    Serial.println(handServo.ReadPos(3));
    Serial.println(handServo.ReadPos(4));
    Serial.println("Beginning to move MIDDLE finger");
    handServo.SyncWritePosEx(MIDDLE, 2, MIDDLE_POSITION_OPEN, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("MIDDLE finger finished moving");
    handServo.SyncWritePosEx(MIDDLE, 2, MIDDLE_POSITION_CLOSE, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("Ending Positions:");
    Serial.println(handServo.ReadPos(3));
    Serial.println(handServo.ReadPos(4));
  }
  else if (index == 2){
    Serial.println(handServo.ReadPos(5));
    Serial.println(handServo.ReadPos(6));
    Serial.println("Beginning to move RING finger");
    handServo.SyncWritePosEx(RING, 2, RING_POSITION_OPEN, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("RING finger finished moving");
    handServo.SyncWritePosEx(RING, 2, RING_POSITION_CLOSE, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("Ending Positions:");
    Serial.println(handServo.ReadPos(5));
    Serial.println(handServo.ReadPos(6));
  }
  else if (index == 3){
    Serial.println(handServo.ReadPos(7));
    Serial.println(handServo.ReadPos(8));
    Serial.println("Beginning to move THUMB");
    handServo.SyncWritePosEx(THUMB, 2, THUMB_POSITION_OPEN, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("THUMB finished moving");
    handServo.SyncWritePosEx(THUMB, 2, THUMB_POSITION_CLOSE, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("Ending Positions:");
    Serial.println(handServo.ReadPos(5));
    Serial.println(handServo.ReadPos(6));
  }
}