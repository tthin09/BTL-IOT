import serial
import time

# Remember to config port
PORT = 'COM22'
BAUDRATE = 115200

ser = serial.Serial(PORT, BAUDRATE, timeout=1)
print(f"Successfully opened serial port {PORT} at {BAUDRATE} baud.")
time.sleep(2)

def send_message_to_serial(message: str):
    message = message.lower()
    message = 'l' if message == 'left' else 'r' if message == 'right' else message
    if message not in ['l', 'r']:
        print("Invalid input. Only pass in direction 'l' or 'r'")
        return
    try:
        ser.write(message.encode('utf-8'))
        print(f"Sent: '{message}'")

        time.sleep(0.1) # Give the ESP32 a moment to respond
        if ser.in_waiting > 0:
            received_data = ser.read(ser.in_waiting).decode('utf-8')
            print(f"Received: '{received_data.strip()}'") # .strip() to remove potential newline characters

    except serial.SerialException as e:
        print(f"Error: Could not open or communicate with serial port {PORT}.")
        print(f"Details: {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    while True:
        user_input = input("Enter your message: ")
        if user_input.lower() == 'q':
            break;
        send_message_to_serial(user_input)
        
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print(f"Serial port {PORT} closed.")
