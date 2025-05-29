#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <ESP32Servo.h>
#include <Ultrasonic.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <OneButton.h>
#include "servo.h"
#include "utils.h"


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
        strncpy(lastMessage, "Added LEFT\0", 16);
        addTaskLeft();
      } else if (incomingChar == 'r') {
        strncpy(lastMessage, "Added RIGHT\0", 16);
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
    servoTurn(0);
    digitalWrite(LED_A, HIGH);
    digitalWrite(LED_B, LOW);
    Serial.println("Trigger task LEFT from queue");
    strncpy(lastMessage, "Triggered LEFT\0", 16);
    break;
    case RIGHT:
    servoTurn(180);
    digitalWrite(LED_A, LOW);
    digitalWrite(LED_B, HIGH);
    Serial.println("Trigger task RIGHT from queue");
    strncpy(lastMessage, "Triggered RIGHT\0", 16);
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