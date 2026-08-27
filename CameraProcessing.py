import cv2
import mediapipe as mp
import time
import serial
import serial.tools.list_ports

startTime = time.time()
motorsActive = False # Initially, motors are off
motorStatus = "Motors: OFF"
textColor = (0, 0, 255)
pinkyIndices = {17, 18, 19, 20}

BAUD_RATE = 115200

videoCapture = cv2.VideoCapture(0)
videoCapture.set(cv2.CAP_PROP_FRAME_WIDTH, 600)
videoCapture.set(cv2.CAP_PROP_FRAME_HEIGHT, 600)

BaseOptions = mp.tasks.BaseOptions
GestureRecognizer = mp.tasks.vision.GestureRecognizer
GestureRecognizerOptions = mp.tasks.vision.GestureRecognizerOptions
VisionRunningMode = mp.tasks.vision.RunningMode

mpHandsConnections = mp.solutions.hands.HAND_CONNECTIONS

options = GestureRecognizerOptions(
    base_options = BaseOptions(
        model_asset_path="gesture_recognizer.task",
        delegate = BaseOptions.Delegate.CPU),
    running_mode = VisionRunningMode.VIDEO,
    num_hands = 1,
    # min_hand_detection_confidence = 0.7 --> Threshold for initial hand detection
)

def find_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "usbserial" in port.device.lower():
            return port.device
    return None

devicePort = find_port()
print(f"Port Found: {devicePort}")
esp32 = serial.Serial(devicePort, BAUD_RATE, timeout=2)

with GestureRecognizer.create_from_options(options) as recognizer:
    while videoCapture.isOpened():
        isRead, frame = videoCapture.read()
        cv2.putText(frame, motorStatus, (50, 50), cv2.FONT_HERSHEY_COMPLEX, 1, textColor, 2, cv2.LINE_AA)
        if isRead:
            RGBFrame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB) # Converting from BGR (OpenCV) to RGB (Mediapipe)
            mpFrame = mp.Image(image_format=mp.ImageFormat.SRGB, data=RGBFrame)
            frameTimestamp = int((time.time() - startTime) * 1000)
            mpResult = recognizer.recognize_for_video(mpFrame, frameTimestamp)
            if mpResult.hand_landmarks:
                for handIndex, handLandmarks in enumerate(mpResult.hand_landmarks):
                    points = []
                    for index, landmark in enumerate(handLandmarks): # Ignores pinky finger (17-20)
                        if index <= 16:
                            x = int(landmark.x * frame.shape[1])
                            y = int(landmark.y * frame.shape[0])
                            points.append((x, y))
                            # cv2.circle(frame, (x, y), 7, (0, 0, 255), -1)
                            cv2.putText(frame, str(index), (x+7, y+2), cv2.FONT_HERSHEY_COMPLEX, 0.6, (255, 0, 0), 2, cv2.LINE_AA)

                    for connection in mpHandsConnections:
                        start, end = connection
                        if start in pinkyIndices or end in pinkyIndices:
                            continue
                        cv2.line(frame, points[start], points[end], (0, 255, 0), 1)

                    if not motorsActive:
                        whichHand = mpResult.handedness[0][0].category_name
                        whichGesture = mpResult.gestures[0][0].category_name
                        gestureScore = mpResult.gestures[0][0].score
                        cv2.putText(frame, "Gesture: " + str(whichGesture), (50, 150), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 0, 0), 2, cv2.LINE_AA)
                        cv2.putText(frame, "Score: " + str(gestureScore), (50, 200), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 0, 0), 2, cv2.LINE_AA)

                        if whichHand == "Right" and whichGesture == "Open_Palm" and gestureScore > 0.70:
                            motorsActive = True
                            motorStatus = "Motors: ON"
                            textColor = (0, 255, 0)
                            esp32.write(b"Motors Active!\n")
                            maxDistIndexf = int((((points[8][0]-points[5][0])**2 + (points[8][1]-points[5][1])**2)**0.5)) / 2

                    if motorsActive:
                        distanceIndexf = int((((points[8][0]-points[5][0])**2 + (points[8][1]-points[5][1])**2)**0.5)) / 2
                        percentBend = round((maxDistIndexf - distanceIndexf) / maxDistIndexf, 3) 
                        if percentBend >= 0:
                            motorBend = str(percentBend)
                            esp32.write((motorBend + "\n").encode("utf-8"))
                        cv2.putText(frame, str(percentBend*100) + "%", (50, 100), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 0, 0), 2, cv2.LINE_AA)
            cv2.imshow("Behold", frame)
            if cv2.waitKey(1) == 113:
                break
        else:
            print("Failed to Read")
            break

videoCapture.release()
cv2.destroyAllWindows()
