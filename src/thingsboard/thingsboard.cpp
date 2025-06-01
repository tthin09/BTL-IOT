#include "thingsboard.h"

WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

void connectWifi() {
  Serial.print("Wifi SSID: " + String(WIFI_SSID) + ", Wifi password: " + String(WIFI_PASSWORD) + "\n");
  Serial.print("Connecting to wifi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Status: ");
    Serial.println(WiFi.status());
    attempts++;
  }
  Serial.println();
  Serial.println("Connected to wifi!");
}

void connectThingsboard() {
  if (!tb.connected()) {
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    const bool successful = tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT);
    if (!successful) {
      Serial.println("Failed to connect to ThingsBoard");
      return;
    }
    
    tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());
    tb.sendAttributeData("rssi", WiFi.RSSI());
    tb.sendAttributeData("channel", WiFi.channel());
    tb.sendAttributeData("bssid", WiFi.BSSIDstr().c_str());
    tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
    tb.sendAttributeData("ssid", WiFi.SSID().c_str());
    Serial.println("Subscribe to ThingsBoard done");
  }
}