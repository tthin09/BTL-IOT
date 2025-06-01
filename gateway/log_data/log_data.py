import csv
import time
import os

def log_data_to_csv(filepath, capture_time, call_model_time, send_serial_time):
    """
    Appends a new row of data (capture_time, call_model_time, send_serial_timec) to a CSV file.
    Creates the file and writes headers if it doesn't exist.
    """
    # Check if the file exists to determine if headers are needed
    file_exists = os.path.exists(filepath)

    try:
        # Open the CSV file in append mode ('a').
        # 'newline=''' is important to prevent extra blank rows.
        with open(filepath, mode='a', newline='') as file:
            writer = csv.writer(file)

            # Write header row only if the file is new
            if not file_exists:
                writer.writerow(['Timestamp', 'Capture Image Time', 'Call AI model Time', 'Send to Serial Time'])
                print(f"Created new CSV file: {filepath} with headers.")

            # Get current timestamp
            timestamp = time.strftime('%Y-%m-%d %H:%M:%S')

            # Write the data row
            writer.writerow([timestamp, capture_time, call_model_time, send_serial_time])
            print(f"Appended data to CSV: {timestamp}, {capture_time}, {call_model_time}, {send_serial_time}")

    except IOError as e:
        print(f"Error writing to CSV file {filepath}: {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")