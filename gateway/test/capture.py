import cv2
import datetime
import time
import os

OUTPUT_FOLDER = "images"
DELAY = 10

def capture_image():
    """
    Captures images from the webcam at a specified interval and saves them
    """
    # Create the output folder if it doesn't exist
    if not os.path.exists(OUTPUT_FOLDER):
        os.makedirs(OUTPUT_FOLDER)
        print(f"Created folder: {OUTPUT_FOLDER}")

    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Error: Could not open webcam. Please ensure it's connected and not in use.")
        return

    print("Press 'q' to quit the capture process.")
    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                print("Error: Could not read frame from webcam. Exiting...")
                break

            timestamp = datetime.datetime.now().strftime("%Y%m%d%H%M%S")
            filename = os.path.join(OUTPUT_FOLDER, f"image_{timestamp}.jpg")

            cv2.imwrite(filename, frame)
            print(f"Captured and saved: {filename}")
            cv2.imshow("Webcam Capture", frame)

            start_time = time.time()
            while (time.time() - start_time) < DELAY:
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    print("'q' pressed. Exiting capture process.")
                    return
                time.sleep(0.1)
    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        cap.release()
        cv2.destroyAllWindows()
        print("Webcam released and windows closed.")

if __name__ == "__main__":
    capture_image()
