from inference_sdk import InferenceHTTPClient
import os
from dotenv import load_dotenv

load_dotenv()

# Roboflow model setting
CLIENT = InferenceHTTPClient(
    api_url=os.getenv("ROBOFLOW_API_URL"),
    api_key=os.getenv("ROBOFLOW_API_KEY")
)

# Camera capture settings
OUTPUT_FOLDER = "images"
DELAY = 10

# Remember to config port
PORT = 'COM22'
BAUDRATE = 115200

