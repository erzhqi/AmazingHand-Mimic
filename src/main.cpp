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

  int16_t THUMB_POSITION_CLOSE[2] = {1068, 3944};
  int16_t THUMB_POSITION_OPEN[2] = {3999, 1184};

  int16_t FINGERS_2D_CLOSE[3][2]{
    {3184, 1101},
    {3176, 835},
    {1068, 3944}
  };
  int16_t FINGERS_2D_OPEN[3][2]{
    {1926, 2169},
    {2040, 2108},
    {3999, 1184}
  };

  int16_t ALL_POSITION_CLOSE[8] = {3044, 1038, 3184, 1101, 3176, 835, 1068, 3944};
  int16_t ALL_POSITION_OPEN[8] = {1875, 2183, 1926, 2169, 2040, 2108, 3999, 1184};


  uint16_t SPEED[2] = {1000, 1000};
  uint8_t ACCELERATION[2] = {50, 50};

void ping_servos();
void finger_movement_test(int index);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Beginning Test");

  // Initializing serial for hand
  handSerial.begin(1000000, SERIAL_8N1, HAND_RX_PIN, HAND_TX_PIN);
  handServo.pSerial = &handSerial;
  delay(200);

  // Initializing serial for arm
  armSerial.begin(1000000, SERIAL_8N1, ARM_RX_PIN, ARM_TX_PIN);
  armServo.pSerial = &armSerial;
  delay(200);

  finger_movement_test(0);
  finger_movement_test(1);
  finger_movement_test(2);

}

void loop() {
  // put your main code here, to run repeatedly:
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
    Serial.println("Beginning to move finger");
    handServo.SyncWritePosEx(MIDDLE, 2, MIDDLE_POSITION_OPEN, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("Finger finished moving");
    handServo.SyncWritePosEx(MIDDLE, 2, MIDDLE_POSITION_CLOSE, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("Ending Positions:");
    Serial.println(handServo.ReadPos(3));
    Serial.println(handServo.ReadPos(4));
  }
  else if (index == 2){
    Serial.println(handServo.ReadPos(5));
    Serial.println(handServo.ReadPos(6));
    Serial.println("Beginning to move finger");
    handServo.SyncWritePosEx(RING, 2, RING_POSITION_OPEN, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("Finger finished moving");
    handServo.SyncWritePosEx(RING, 2, RING_POSITION_CLOSE, SPEED, ACCELERATION);
    delay(2000);
    Serial.println("Ending Positions:");
    Serial.println(handServo.ReadPos(5));
    Serial.println(handServo.ReadPos(6));
  }
}