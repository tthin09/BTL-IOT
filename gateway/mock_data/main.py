import time
from utils import *
import pandas as pd
import random

import paho.mqtt.client as mqttclient
import json

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
    filepath = "/../log_data/execution_time.csv"
    df = pd.read_csv(filepath)
    data = df["Call AI model Time"]
    timestamp = df["Timestamp"]
    new_df = pd.DataFrame({
            "Timestamp": timestamp,
            "Call AI Model Time": data
        })
        
    # Define the path for the new CSV file
    output_filepath = "new_data.csv"
    new_df.to_csv(output_filepath, index=False)
    types = ["organic", "inorganic"]
    while True:
        for call_time in data:
            choice = random.choice(types)
            data_sent = {"timeToQuery": call_time, "wasteType": choice}
            client.publish('v1/devices/me/telemetry', json.dumps(data_sent), 1)
            print(f"timeToQuery: {call_time}, wasteType: {choice}")
            time.sleep(3)