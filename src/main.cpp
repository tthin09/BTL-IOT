#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <Ultrasonic.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <OneButton.h>

#include "servo/servo.h"
#include "thingsboard/thingsboard.h"
#include "utils.h"


const int BAUDRATE = 115200;

// Led
const int LED_A = 18;
const int LED_B = 19;

// Button
const int buttonRightPin = 16;
const int buttonLeftPin = 17;
OneButton buttonRight(buttonRightPin, true);
OneButton buttonLeft(buttonLeftPin, true);

// Ultrasonic sensor
const int entrySensorTrigPin = 32;
const int entrySensorEchoPin = 33;
const int exitSensorTrigPin = 25;
const int exitSensorEchoPin = 26;
Ultrasonic entrySensor(entrySensorTrigPin, entrySensorEchoPin);
Ultrasonic exitSensor(exitSensorTrigPin, exitSensorEchoPin);
const int DISTANCE_THRESHOLD = 30; // cm

// LCD monitor
LiquidCrystal_I2C lcd(0x21, 16, 2);
char lastMessage[17];

// Servo
QueueHandle_t servoTasks;

// HELPER FUNCTION DECLARE
void lcdUpdate();
void stopConveyorBelt();
void continueConveyorBelt();

// =============================================================

void entrySensorTask(void *pvParameters) {
  while (1) {
    vTaskDelay(10);
    double distance = entrySensor.read(CM);
    if (distance <= DISTANCE_THRESHOLD) {
      Serial.println("New waste detected!");
      strncpy(lastMessage, "Waste detected\0", 16);
      lcdUpdate();
      vTaskDelay(1000);
    }
  }
}

void exitSensorTask(void *pvParameters) {
  while (1) {
    vTaskDelay(10);
    double distance = exitSensor.read(CM);
    if (distance <= DISTANCE_THRESHOLD) {
      Serial.println("Ultrasonic distance: " + String(distance) + " cm");
      if (uxQueueMessagesWaiting(servoTasks) <= 0) {
        Serial.println("Waste came to exit point but no tasks is in queue. Waiting...");
        strncpy(lastMessage, "Wait for task...\0", 16);
        lcdUpdate();
        stopConveyorBelt();
        while (uxQueueMessagesWaiting(servoTasks) <= 0) {
          vTaskDelay(1000);
        }
        Serial.println("Found new task to exit!");
        continueConveyorBelt();
      }
      ServoTaskType task;
      int result = xQueueReceive(servoTasks, &task, 0);
      if (result != pdPASS) {
        Serial.println("Failed to get task from queue");
        continue;
      }
      triggerServo(task);
      if (task == LEFT) {
        digitalWrite(LED_A, HIGH);
        digitalWrite(LED_B, LOW);
        Serial.println("Trigger task LEFT from queue");
        strncpy(lastMessage, "Triggered LEFT\0", 16);
        if (tb.sendTelemetryData("wasteType", "organic")) {
          Serial.println("Send telemetry 'organic' successfully");
        }
      } else if (task == RIGHT) {
        digitalWrite(LED_A, HIGH);
        digitalWrite(LED_B, LOW);
        Serial.println("Trigger task RIGHT from queue");
        strncpy(lastMessage, "Triggered RIGHT\0", 16);
        if (tb.sendTelemetryData("wasteType", "organic")) {
          Serial.println("Send telemetry 'organic' successfully");
        }
      }
      printQueueSize(servoTasks);
      lcdUpdate();
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
        addTaskLeft(servoTasks);
        strncpy(lastMessage, "Added LEFT\0", 16);
      } else if (incomingChar == 'r') {
        addTaskRight(servoTasks);
        strncpy(lastMessage, "Added RIGHT\0", 16);
      } else {
        strncpy(lastMessage, "UNKNOWN\0", 16);
      }
      lcdUpdate();
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to yield CPU time
  }
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
  
  entrySensor.setTimeout(40000UL);
  exitSensor.setTimeout(40000UL);
  
  Wire.begin();
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");

  xTaskCreate(entrySensorTask, "Entry sensor task", 4096, NULL, 1, NULL);
  xTaskCreate(exitSensorTask, "Exit sensor task", 4096, NULL, 1, NULL);
  xTaskCreate(serialListenTask, "Serial listen task", 8192, NULL, 2, NULL);

  continueConveyorBelt();
}

// =============================================================

void lcdUpdate() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Queue size: ");
  lcd.setCursor(12, 0);
  lcd.print(uxQueueMessagesWaiting(servoTasks));
  
  lcd.setCursor(0, 1);
  lcd.print(lastMessage);
}

void stopConveyorBelt() {
  digitalWrite(LED_A, HIGH);
  digitalWrite(LED_B, HIGH);
}

void continueConveyorBelt() {
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
}

// =============================================================

void loop() {
  // FreeRTOS handle all tasks
}