import cv2
import mediapipe as mp
import time

startTime = time.time()

videoCapture = cv2.VideoCapture(0)
videoCapture.set(cv2.CAP_PROP_FRAME_WIDTH, 600)
videoCapture.set(cv2.CAP_PROP_FRAME_HEIGHT, 600)

BaseOptions = mp.tasks.BaseOptions
HandLandmarker = mp.tasks.vision.HandLandmarker
HandLandmarkerOptions = mp.tasks.vision.HandLandmarkerOptions
VisionRunningMode = mp.tasks.vision.RunningMode

mpHandsConnections = mp.solutions.hands.HAND_CONNECTIONS

options = HandLandmarkerOptions(
    base_options = BaseOptions(
        model_asset_path="hand_landmarker.task",
        delegate = BaseOptions.Delegate.CPU),
    running_mode = VisionRunningMode.VIDEO,
    num_hands = 1,
    # min_hand_detection_confidence = 0.7 --> Threshold for initial hand detection
)

with HandLandmarker.create_from_options(options) as landmarker:

    while videoCapture.isOpened():
        isRead, frame = videoCapture.read()
        if isRead:
            RGBFrame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB) # Converting from BGR (OpenCV) to RGB (Mediapipe)
            mpFrame = mp.Image(image_format=mp.ImageFormat.SRGB, data=RGBFrame)
            frameTimestamp = int((time.time() - startTime) * 1000)
            mpResult = landmarker.detect_for_video(mpFrame, frameTimestamp)

            if mpResult.hand_landmarks:
                for handLandmarks in mpResult.hand_landmarks:
                    points = []
                    for index, landmark in enumerate(handLandmarks): # Ignores pinky finger (17-20)
                        if index <= 16:
                            x = int(landmark.x * frame.shape[1])
                            y = int(landmark.y * frame.shape[0])
                            points.append((x, y))
                            cv2.circle(frame, (x, y), 7, (0, 0, 255), -1)
                            print(points[0])
                            print(points[0][0])
                            print(points[0][1])

                    pinkyIndices = {17, 19, 19, 20}
                    for connection in mpHandsConnections:
                        start, end = connection
                        if start in pinkyIndices or end in pinkyIndices:
                            continue
                        cv2.line(frame, points[start], points[end], (0, 255, 0), 1)

            cv2.imshow("Behold", frame)
            if cv2.waitKey(1) == 113:
                break
        else:
            print("Failed to Read")
            break

videoCapture.release()
cv2.destroyAllWindows()
