import serial
import time

# Remember to config port
PORT = 'COM22'
BAUDRATE = 115200

ser = serial.Serial(PORT, BAUDRATE, timeout=1)
print(f"Successfully opened serial port {PORT} at {BAUDRATE} baud.")
time.sleep(2)

def send_message_to_serial(message):
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
        print("Please ensure:")
        print("1. The correct port name is used.")
        print("2. The device is connected and powered on.")
        print("3. No other program is using the serial port.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
    finally:
        # Always close the serial port when done
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print(f"Serial port {PORT} closed.")

if __name__ == "__main__":
    while True:
        user_input = input("Enter your message: ")
        send_message_to_serial(user_input)