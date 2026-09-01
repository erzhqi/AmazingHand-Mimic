# AmazingHand-Mimic
Vision-controlled robotic hand project that tracks the user's right hand in real time through the Python OpenCV and MediaPipe libraries. Data is then sent through PySerial to an ESP32 to control the servos on an Amazing Hand by Pollen Robotics.

## Hardware
- ESP32 (Freenove WROVER)
- Amazing Hand (STS3032 servos)
- FE-URT-2 Board
- Power supply, breadboard (Power distribution to the servos)

## Software Setup

## main.cpp
Open the project in PlatformIO and upload to your ESP32

## CameraProcessing.py
1. Create a Python Virtual Environment (Version 3.11)
```bash
   python3.11 -m venv venv
   source venv/bin/activate
   pip install -r requirements.txt
```

2. Download the gesture recognizer
```bash
   curl -o gesture_recognizer.task https://storage.googleapis.com/mediapipe-models/gesture_recognizer/gesture_recognizer/float16/1/gesture_recognizer.task
```

3. Run the python script

## How it works
1. Python captures webcam frames through OpenCV and feeds them into MediaPipe's Gesture Recognizer
2. When the tracked hand is identified as being a right hand and the gesture detected is "Open_Palm", a message is sent through serial to initialize the servos
3. Per-finger bend percentages are then calculated from distances between keypoints on the detected hand in comparison to an initial distance when the open right hand is detected
4. These values are sent through serial to the ESP32, where they are parsed and written to the servos via "SyncWritePos"

## Notes
To close the webcam window, click the 'q' key at any time

## Status
Initial version: All four fingers move according to your hand
Future Possible Fixes:
1. After initial detection by right hand, you can switch to control with your left hand
2. Maybe add side to side movement for fingers