#ifndef THINGSBOARD_H
#define THINGSBOARD_H

#include <Arduino.h>
#include <Wifi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>

// Wifi config
constexpr char WIFI_SSID[] = "ACLAB";
constexpr char WIFI_PASSWORD[] = "ACLAB2023";

// Thingsboard setup
constexpr char TOKEN[] = "crr1n3ilqu162iug4e9h";
constexpr char THINGSBOARD_SERVER[] = "app.coreiot.io";
constexpr uint16_t THINGSBOARD_PORT = 1883U;

constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;

// Objects
WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

void connectThingsboard();
void connectWifi();

#endif