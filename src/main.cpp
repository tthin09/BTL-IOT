#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <ESP32Servo.h>
#include <Ultrasonic.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <OneButton.h>
#include <Wifi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include "servo.h"
#include "utils.h"

//=====================================

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

//=====================================

const int BAUDRATE = 115200;

// Led
const int LED_A = 18;
const int LED_B = 19;

// Servo
Servo servo;
const int servoPin = 27;
int currentServoDir = 0;
QueueHandle_t servoTasks;

// Button
const int buttonRightPin = 16;
const int buttonLeftPin = 17;
OneButton buttonRight(buttonRightPin, true);
OneButton buttonLeft(buttonLeftPin, true);
void addTaskLeft();
void addTaskRight();

// Ultrasonic sensor
const int trigPin = 25;
const int echoPin = 26;
Ultrasonic ultrasonic(trigPin, echoPin);
const int DISTANCE_THRESHOLD = 30; // cm

// LCD monitor
LiquidCrystal_I2C lcd(0x21, 16, 2);
char lastMessage[17];

// Helper function for servo
void servoTurn(int direction);
void servoAddTask(ServoTaskType& task);
void triggerServo();

void ultrasonicTask(void *pvParameters) {
  while (1) {
    vTaskDelay(10);
    double distance = ultrasonic.read(CM);
    if (distance <= DISTANCE_THRESHOLD) {
      Serial.println("Ultrasonic distance: " + String(distance) + " cm");
      triggerServo();
      vTaskDelay(1500);
    }
  }
}

void buttonTask(void *pvParameters) {
  while (1) {
    buttonLeft.tick();
    buttonRight.tick();
    vTaskDelay(10);
  }
}

void serialListenTask(void *pvParameters) {
  Serial.println("Serial Listen Task started.");
  while (1) {
    if (Serial.available()) { // Check if there's any data in the serial buffer
      char incomingChar = Serial.read(); // Read the incoming character
      Serial.print("Received from serial: ");
      Serial.println(incomingChar); // Print it back to serial
      if (incomingChar == 'l') {
        addTaskLeft();
      } else if (incomingChar == 'r') {
        addTaskRight();
      } else {
        strncpy(lastMessage, "UNKNOWN\0", 16);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to yield CPU time
  }
}

void lcdUpdate() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Queue size: ");
  lcd.setCursor(12, 0);
  lcd.print(uxQueueMessagesWaiting(servoTasks));

  lcd.setCursor(0, 1);
  lcd.print(lastMessage);
}

// =============================================================
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

void wifiTask(void *pvParameters)
{
  connectWifi();
  while (1)
  {
    vTaskDelay(5000);
    if (WiFi.status() != WL_CONNECTED)
    {
      connectWifi();
    }
  }
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

void connectThingsboardTask(void *pvParameters) {
  while (1) {
    connectThingsboard();
    vTaskDelay(5000);
  }
}

void thingsboardTask(void *pvParameters) {
  while (1) {
    tb.loop();
    vTaskDelay(10);
  }
}
// =============================================================

void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUDRATE);
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
  
  servoTasks = xQueueCreate(40, sizeof(ServoTaskType));
  servo.attach(servoPin);
  servo.write(0);
  
  buttonLeft.attachClick(addTaskLeft);
  buttonRight.attachClick(addTaskRight);
  buttonLeft.attachDoubleClick(triggerServo);
  
  ultrasonic.setTimeout(40000UL);
  
  Wire.begin();
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0); // Đặt con trỏ về đầu dòng 0
  lcd.print("System Ready!"); // In thông báo khởi động

  connectWifi();
  connectThingsboard();
  
  xTaskCreate(wifiTask, "Wifi task", 8192, NULL, 1, NULL);
  xTaskCreate(connectThingsboardTask, "Connect Thingsboard task", 8192, NULL, 1, NULL);
  xTaskCreate(thingsboardTask, "Thingsboard running task", 8192, NULL, 1, NULL);
  xTaskCreate(ultrasonicTask, "Ultrasonic task", 8192, NULL, 1, NULL);
  xTaskCreate(buttonTask, "Button task", 4096, NULL, 1, NULL);
  xTaskCreate(serialListenTask, "Serial listen task", 8192, NULL, 2, NULL);
}


// =======================================================
// HELPER FUNCTIONS
// ========================================================

void addTaskLeft() {
  ServoTaskType task = LEFT;
  int result = xQueueSend(servoTasks, &task, 0);
  if (result != pdPASS) {
    Serial.println("Failed to send task to servo queue");
  } else {
    Serial.println("Added task LEFT successfully");
    strncpy(lastMessage, "Added LEFT\0", 16);
    printQueueSize(servoTasks);
  }
  lcdUpdate();
}

void addTaskRight() {
  ServoTaskType task = RIGHT;
  int result = xQueueSend(servoTasks, &task, 0);
  if (result != pdPASS) {
    Serial.println("Failed to send task to servo queue");
  } else {
    Serial.println("Added task RIGHT successfully");
    strncpy(lastMessage, "Added RIGHT\0", 16);
    printQueueSize(servoTasks);
  }
  lcdUpdate();
}

// this function will be called when the button started long pressed.
void triggerServo() {
  ServoTaskType task;
  int result = xQueueReceive(servoTasks, &task, 0);
  if (result != pdPASS) {
    Serial.println("Failed to get task from queue");
    return;
  }
  switch (task) {
    case LEFT:
      int left_degree = 180;
      servoTurn(left_degree);
      digitalWrite(LED_A, HIGH);
      digitalWrite(LED_B, LOW);
      Serial.println("Trigger task LEFT from queue");
      strncpy(lastMessage, "Triggered LEFT\0", 16);
      if (tb.sendTelemetryData("trigger", "LEFT")) {
        tb.sendTelemetryData("light", 30);
        Serial.println("Send telemetry LEFT successfully");
      }
      break;
    case RIGHT:
      int right_degree = 0;
      servoTurn(right_degree);
      digitalWrite(LED_A, LOW);
      digitalWrite(LED_B, HIGH);
      Serial.println("Trigger task RIGHT from queue");
      strncpy(lastMessage, "Triggered RIGHT\0", 16);
      if (tb.sendTelemetryData("trigger", "RIGHT")) {
        tb.sendTelemetryData("light", 30);
        Serial.println("Send telemetry RIGHT successfully");
      }
      break;
      default:
      Serial.println("Unknown command");
      digitalWrite(LED_A, LOW);
      digitalWrite(LED_B, LOW);
      break;
    }
    printQueueSize(servoTasks);
    lcdUpdate();
  }
  
void servoTurn(int direction) {
  Serial.println("Write servo at " + String(direction) + " degrees");
  servo.write(direction);
}

void loop() {
  // FreeRTOS handle all tasks
}