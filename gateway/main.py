import cv2
import datetime
import time
import os
from dotenv import load_dotenv
import serial
from utils import *
from log_data.log_data import log_data_to_csv

import paho.mqtt.client as mqttclient
import json

# Log model Execution time
DATA_FILEPATH = "log_data/execution_time.csv"

# Serial settings
ser = serial.Serial(PORT, BAUDRATE, timeout=1)
print(f"Successfully opened serial port {PORT} at {BAUDRATE} baud.")

# Create the output folder if it doesn't exist
if not os.path.exists(OUTPUT_FOLDER):
    os.makedirs(OUTPUT_FOLDER)
    print(f"Created folder: {OUTPUT_FOLDER}")

print("Opening webcam...")
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Could not open webcam. Please ensure it's connected and not in use.")

def capture_image() -> str:
    """
    Captures images from the webcam at a specified interval and saves them
    """
    try:
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
    return result["top"]
    
def send_message_to_serial(wasteType: str) -> bool:
    lib = {"organic": "l",
           "inorganic": "r",
           "UNKNOWN": "u"}
    message = lib[wasteType]
    try:
        ser.write(message.encode('utf-8'))
        print(f"Sent: '{message}'")
        return True

    except serial.SerialException as e:
        print(f"Error: Could not open or communicate with serial port {PORT}.")
        print(f"Details: {e}")
        return False
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        return False
    
# ============================================================
# ThingsBoard
# ============================================================

BROKER_ADDRESS = "app.coreiot.io"
BROKER_PORT = 1883
ACCESS_TOKEN = "ctn25wq8yimsvo1ytdwq"
ACCESS_USERNAME = "ecosort"
CLIENT_ID = "EcoSort_v1"

def subscribed(client, userdata, mid, granted_qos):
    print("Subscribed...")

def recv_message(client, userdata, message):
    print("Received: ", message.payload.decode("utf-8"))
    temp_data = {'value': True}
    try:
        jsonobj = json.loads(message.payload)
        if jsonobj['method'] == "setValue":
            temp_data['value'] = jsonobj['params']
            client.publish('v1/devices/me/attributes', json.dumps(temp_data), 1)
    except:
        pass

def connected(client, usedata, flags, rc):
    if rc == 0:
        print("Connected successfully!!")
        client.subscribe("v1/devices/me/rpc/request/+")
    else:
        print("Connection is failed")

client = mqttclient.Client(CLIENT_ID)
client.username_pw_set(ACCESS_USERNAME, ACCESS_TOKEN)

client.on_connect = connected
client.connect(BROKER_ADDRESS, BROKER_PORT)
client.loop_start()

client.on_subscribe = subscribed
client.on_message = recv_message


if __name__ == "__main__":
    print("Press SPACE to capture an image manually")
    print("Press 'q' to quit the capture process.")
    running = True
    while running:
        if ser.in_waiting > 0:
            received_data = ser.read(ser.in_waiting).decode('utf-8').strip()
            if received_data == "New waste detected!":
                print("\nCapturing image...")
                start_time = time.time()
                image_path = capture_image()
                capture_time = time.time() - start_time
                
                start_time = time.time()
                wasteType = get_prediction(image_path)
                call_model_time = time.time() - start_time
                
                start_time = time.time()
                response = send_message_to_serial(wasteType)
                send_serial_time = time.time() - start_time
                log_data_to_csv(DATA_FILEPATH, capture_time, call_model_time, send_serial_time)
                
                print(f"Result: {wasteType}")
                data = {"wasteType": wasteType}
                client.publish('v1/devices/me/telemetry', json.dumps(data), 1)
                print(f"Time: {datetime.datetime.now()}, Waste type: {wasteType}")
            
            
        if cv2.waitKey(1) & 0xFF == ord('q'):
            print("'q' pressed. Exiting capture process.")
            running = False
            break
        elif cv2.waitKey(1) & 0xFF == ord(' '):
            break
        time.sleep(0.1)

    cap.release()
    cv2.destroyAllWindows()
    print("Webcam released and windows closed.")