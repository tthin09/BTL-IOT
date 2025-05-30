print("Hello Core IOT")
import paho.mqtt.client as mqttclient
import time
import json
import serial

BROKER_ADDRESS = "app.coreiot.io"
PORT = 1883
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
client.connect(BROKER_ADDRESS, 1883)
client.loop_start()

client.on_subscribe = subscribed
client.on_message = recv_message

temp = 30
humidity = 50
light_intesity = 100
counter = 0


while True:
    print(f"Temperature: {temp:.2f} *C, Humidity: {humidity:.2f}%")
    collect_data = {'temperature': temp, 'humidity': humidity}
    info = client.publish('v1/devices/me/telemetry', json.dumps(collect_data), 1)
    rc, message_id = info
    print(f"rc: {rc}, message_id: {message_id}")

    temp += 1
    humidity += 1
    time.sleep(3)