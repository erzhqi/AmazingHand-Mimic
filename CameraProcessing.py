"""
    CameraProcessing.py
    Erik Zhu
    August 31, 2026
    Vision side of this project. Uses OpenCV to read frames from a webcam and
    runs the MediaPipe Gesture Recognizer landmark to compute bend percentages
    for each finger. These values are then sent as a comma separated string to the 
    ESP32 over serial so the AmazingHand can mimic the movements of my hand.
"""
import cv2
import mediapipe as mp
import time # For timestampping frames
import serial
import serial.tools.list_ports

startTime = time.time()
motorsActive = False # Initially, motors are off
motorStatus = "Motors: OFF"
textColor = (0, 0, 255)
pinkyIndices = {17, 18, 19, 20}

BAUD_RATE = 115200 # (ESP: 115200, Arduino: 115200 OR 9600 in some cases)

# Setting up video capture object
videoCapture = cv2.VideoCapture(0)
videoCapture.set(cv2.CAP_PROP_FRAME_WIDTH, 600)
videoCapture.set(cv2.CAP_PROP_FRAME_HEIGHT, 600)

# Configuring options for MediaPipe and the gesture recognizer task
BaseOptions = mp.tasks.BaseOptions
GestureRecognizer = mp.tasks.vision.GestureRecognizer
GestureRecognizerOptions = mp.tasks.vision.GestureRecognizerOptions
VisionRunningMode = mp.tasks.vision.RunningMode

mpHandsConnections = mp.solutions.hands.HAND_CONNECTIONS

options = GestureRecognizerOptions(
    base_options = BaseOptions(
        model_asset_path="gesture_recognizer.task",
        delegate = BaseOptions.Delegate.CPU), # Have to manually delegate CPU space for MacOS,
    running_mode = VisionRunningMode.VIDEO,
    num_hands = 1 # Only allows for tracking one hand at once,
)

# Searching for appropriate port for serial
def find_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "usbserial" in port.device.lower(): # MacOS specific format for ports
            return port.device
    return None

devicePort = find_port()
print(f"Port Found: {devicePort}")
# Setting up the ESP32 port 
esp32 = serial.Serial(devicePort, BAUD_RATE, timeout=2)

# Creating a recognizer object with our configured options
with GestureRecognizer.create_from_options(options) as recognizer:
    # Runs a loop while the camera is on
    while videoCapture.isOpened():
        # Returns a variable "isRead" if the camera is reading, as well as the current frame (NumPy array)
        isRead, frame = videoCapture.read()
        # Motor status
        cv2.putText(frame, motorStatus, (50, 50), cv2.FONT_HERSHEY_COMPLEX, 1, textColor, 2, cv2.LINE_AA)
        if isRead: # Runs if the camera is working
            RGBFrame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB) # Converting from BGR (OpenCV) to RGB (Mediapipe)
            mpFrame = mp.Image(image_format=mp.ImageFormat.SRGB, data=RGBFrame)
            frameTimestamp = int((time.time() - startTime) * 1000) # Timestamp associated with a frame
            mpResult = recognizer.recognize_for_video(mpFrame, frameTimestamp) 
            if mpResult.hand_landmarks: # If hand(s) are detected in a given frame
                for handIndex, handLandmarks in enumerate(mpResult.hand_landmarks):
                    points = []
                    for index, landmark in enumerate(handLandmarks): # Ignores pinky finger (17-20)
                        if index <= 16:
                            x = int(landmark.x * frame.shape[1])
                            y = int(landmark.y * frame.shape[0])
                            points.append((x, y)) # Adding coordinates of each keypoint on hand to array
                            # cv2.circle(frame, (x, y), 7, (0, 0, 255), -1)
                            
                            # Printing keypoint number next to each keypoint
                            cv2.putText(frame, str(index), (x+7, y+2), cv2.FONT_HERSHEY_COMPLEX, 0.6, (255, 0, 0), 2, cv2.LINE_AA)

                    # Drawing in each connection between keypoints
                    for connection in mpHandsConnections:
                        start, end = connection
                        #Ignoring connections that involve the pinky finge
                        if start in pinkyIndices or end in pinkyIndices:
                            continue
                        cv2.line(frame, points[start], points[end], (0, 255, 0), 1)

                    if not motorsActive: # Initial code to detect an "Open_Palm" gesture by the right hand
                        whichHand = mpResult.handedness[0][0].category_name
                        whichGesture = mpResult.gestures[0][0].category_name
                        gestureScore = mpResult.gestures[0][0].score

                        if whichHand == "Right" and whichGesture == "Open_Palm" and gestureScore > 0.70:
                            motorsActive = True
                            motorStatus = "Motors: ON"
                            textColor = (0, 255, 0)
                            esp32.write(b"Motors Active!\n")

                            # Initializing a maximum distance between keypoints to serve as the baseline for
                            # a completely open hand
                            maxDistIndexf = int((((points[8][0]-points[5][0])**2 + (points[8][1]-points[5][1])**2)**0.5)) / 2
                            maxDistMiddlef = int((((points[12][0]-points[9][0])**2 + (points[12][1]-points[9][1])**2)**0.5)) / 2
                            maxDistRingf = int((((points[16][0]-points[13][0])**2 + (points[16][1]-points[13][1])**2)**0.5)) / 2
                            maxDistThumb = int((((points[4][0]-points[0][0])**2 + (points[4][1]-points[0][1])**2)**0.5)) / 2

                    if motorsActive:
                        # Calculating current distance between keypoints
                        distanceIndexf = int((((points[8][0]-points[5][0])**2 + (points[8][1]-points[5][1])**2)**0.5)) / 2
                        distanceMiddlef = int((((points[12][0]-points[9][0])**2 + (points[12][1]-points[9][1])**2)**0.5)) / 2
                        distanceRingf = int((((points[16][0]-points[13][0])**2 + (points[16][1]-points[13][1])**2)**0.5)) / 2
                        distanceThumb = int((((points[4][0]-points[0][0])**2 + (points[4][1]-points[0][1])**2)**0.5)) / 2

                        # Comparing to maximum distance and calculating a bend percentage
                        percentBendIndex = round((maxDistIndexf - distanceIndexf) / maxDistIndexf, 2) 
                        percentBendMiddle = round((maxDistMiddlef - distanceMiddlef) / maxDistMiddlef, 2) 
                        percentBendRing = round((maxDistRingf - distanceRingf) / maxDistRingf, 2) 
                        percentBendThumb = round((maxDistThumb - distanceThumb) / maxDistThumb, 2)

                        fingerBendList = [percentBendIndex, percentBendMiddle, percentBendRing, percentBendThumb]
                        motorBend = ""
                        for i in range(len(fingerBendList)):
                            if fingerBendList[i] < 0: # Normalizing values to 0 if they become negative
                                fingerBendList[i] = 0
                            motorBend += str(fingerBendList[i]) + "," # Adding to string that is sent to ESP32
                        esp32.write((motorBend + "\n").encode("utf-8"))
                        cv2.putText(frame, "Index Bend: " + str(fingerBendList[0]*100) + "%", (50, 100), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 0, 0), 2, cv2.LINE_AA)
                        cv2.putText(frame, "Middle Bend: " + str(fingerBendList[1]*100) + "%", (50, 150), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 0, 0), 2, cv2.LINE_AA)
                        cv2.putText(frame, "Ring Bend: " + str(fingerBendList[2]*100) + "%", (50, 200), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 0, 0), 2, cv2.LINE_AA)
                        cv2.putText(frame, "Thumb Bend: " + str(fingerBendList[3]*100) + "%", (50, 250), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 0, 0), 2, cv2.LINE_AA)
            # Display camera window
            cv2.imshow("Amazing Hand Mimic", frame)
            if cv2.waitKey(1) == 113: # Exits if 'q' is pressed
                break
        else:
            print("Failed to Read")
            break

# Destroying the videoCapture instance and stops execution of OpenCV
videoCapture.release()
cv2.destroyAllWindows()
