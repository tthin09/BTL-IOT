import cv2
import datetime
import time
import os
from dotenv import load_dotenv
import serial
from utils import *

# Serial settings
ser = serial.Serial(PORT, BAUDRATE, timeout=1)
print(f"Successfully opened serial port {PORT} at {BAUDRATE} baud.")

# Create the output folder if it doesn't exist
if not os.path.exists(OUTPUT_FOLDER):
    os.makedirs(OUTPUT_FOLDER)
    print(f"Created folder: {OUTPUT_FOLDER}")

cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Could not open webcam. Please ensure it's connected and not in use.")

def capture_image() -> str:
    """
    Captures images from the webcam at a specified interval and saves them
    """
    try:
        print("Press 'q' to quit the capture process.")
        ret, frame = cap.read()
        if not ret:
            print("Error: Could not read frame from webcam. Exiting...")
            return None

        timestamp = datetime.datetime.now().strftime("%Y%m%d%H%M%S")
        filename = f"image_{timestamp}.jpg"
        image_path = os.path.join(OUTPUT_FOLDER, filename)

        cv2.imwrite(image_path, frame)
        print(f"Captured and saved: {image_path}")
        cv2.imshow("Webcam Capture", frame)
        return image_path
    except Exception as e:
        print(f"An error occurred: {e}")
        
def get_prediction(image_path: str) -> str:
    result = CLIENT.infer(image_path, model_id=os.getenv("ROBOFLOW_MODEL_ID"))
    if result["top"] == "organic":
        return "LEFT"
    elif result["top"] == "inorganic":
        return "RIGHT"
    else:
        return "UNKNOWN"
    
def send_message_to_serial(message: str) -> bool:
    lib = {"LEFT": "l",
           "RIGHT": "r",
           "UNKNOWN": "u"}
    message = lib[message]
    try:
        ser.write(message.encode('utf-8'))
        print(f"Sent: '{message}'")

        time.sleep(0.1) # Give the ESP32 a moment to respond
        if ser.in_waiting > 0:
            received_data = ser.read(ser.in_waiting).decode('utf-8')
            print(f"Received: '{received_data.strip()}'") # .strip() to remove potential newline characters
            return True

    except serial.SerialException as e:
        print(f"Error: Could not open or communicate with serial port {PORT}.")
        print(f"Details: {e}")
        return False
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        return False
        

if __name__ == "__main__":
    running = True
    while running:
        image_path = capture_image()
        prediction_result = get_prediction(image_path)
        result = send_message_to_serial(prediction_result)
        
        start_time = time.time()
        while (time.time() - start_time) < DELAY:
            if cv2.waitKey(1) & 0xFF == ord('q'):
                print("'q' pressed. Exiting capture process.")
                running = False
                break
            time.sleep(0.1)
    
    cap.release()
    cv2.destroyAllWindows()
    print("Webcam released and windows closed.")