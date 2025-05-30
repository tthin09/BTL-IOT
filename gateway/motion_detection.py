import cv2
import numpy as np
import time

def motion_detector_webcam():
    """
    It uses Background Subtraction (MOG2) to detect motion.
    When motion is detected, it draws a bounding box around the moving object.
    Press 'q' to quit.
    """

    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Error: Could not open webcam.")
        return

    # Initialize Background Subtractor (MOG2)
    fgbg = cv2.createBackgroundSubtractorMOG2(history=500, varThreshold=16, detectShadows=False)

    MIN_CONTOUR_AREA = 15000  # Minimum area of detected contours to be considered motion
    MOTION_COLOR = (0, 255, 0) # Green color for bounding boxes
    TEXT_COLOR = (0, 0, 255)   # Red color for text
    FONT = cv2.FONT_HERSHEY_SIMPLEX
    FONT_SCALE = 0.7
    THICKNESS = 2

    print("Motion detection started. Press 'q' to quit.")

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Error: Failed to grab frame. Exiting...")
            break

        # Flip the frame horizontally (useful for webcams)
        frame = cv2.flip(frame, 1)

        # This mask will show white pixels where motion is detected and black elsewhere.
        fgmask = fgbg.apply(frame)

        # Apply morphological operations to clean up the foreground mask
        # noise reduction (erosions) and filling small holes (dilations)
        fgmask = cv2.erode(fgmask, None, iterations=2)
        fgmask = cv2.dilate(fgmask, None, iterations=2)

        # Find contours in the foreground mask
        # cv2.RETR_EXTERNAL retrieves only the extreme outer contours
        # cv2.CHAIN_APPROX_SIMPLE compresses horizontal, vertical, and diagonal segments
        contours, _ = cv2.findContours(fgmask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        motion_detected = False
        for contour in contours:
            # If the contour area is too small, discard it (likely noise)
            if cv2.contourArea(contour) < MIN_CONTOUR_AREA:
                continue

            # A significant contour means motion is detected
            motion_detected = True

            # Get the bounding box for the contour
            (x, y, w, h) = cv2.boundingRect(contour)

            # Draw the bounding box on the original frame
            cv2.rectangle(frame, (x, y), (x + w, y + h), MOTION_COLOR, THICKNESS)

            # Optional: Display the area of the detected motion
            # cv2.putText(frame, f"Area: {cv2.contourArea(contour):.0f}", (x, y - 10), FONT, FONT_SCALE, MOTION_COLOR, THICKNESS)

        # Display motion status text
        if motion_detected:
            status_text = "Status: Motion Detected!"
        else:
            status_text = "Status: No Motion"

        cv2.putText(frame, status_text, (10, 30), FONT, FONT_SCALE, TEXT_COLOR, THICKNESS)

        # Display the original frame with detections
        cv2.imshow("Webcam Feed with Motion Detection", frame)

        # Display the foreground mask (useful for debugging)
        cv2.imshow("Foreground Mask", fgmask)

        # Press 'q' to quit the loop
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Release the webcam and destroy all OpenCV windows
    cap.release()
    cv2.destroyAllWindows()
    print("Motion detection stopped.")

if __name__ == "__main__":
    motion_detector_webcam()