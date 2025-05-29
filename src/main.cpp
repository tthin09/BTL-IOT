#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <ESP32Servo.h>
#include <Ultrasonic.h>
#include <OneButton.h>
#include "servo.h"
#include "utils.h"

#define SERVO_LEFT = 1
#define SERVO_RIGHT = 2

const int BAUDRATE = 115200;

// Led
const int LED_A = 18;
const int LED_B = 19;

// Servo
Servo servo;
const int servoPin = 27;
int currentServoDir = 0;
QueueHandle_t servoTasks;

// button
const int buttonRightPin = 16;
const int buttonLeftPin = 17;
OneButton buttonRight(buttonRightPin, true);
OneButton buttonLeft(buttonLeftPin, true);
void handleButtonLeft();
void handleButtonRight();

// Ultrasonic sensor
const int trigPin = 25;
const int echoPin = 26;
Ultrasonic ultrasonic(trigPin, echoPin);
const int DISTANCE_THRESHOLD = 30; // cm

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
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to yield CPU time
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUDRATE);
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);

  servoTasks = xQueueCreate(40, sizeof(ServoTaskType));
  servo.attach(servoPin);
  servo.write(0);
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);

  buttonLeft.attachClick(handleButtonLeft);
  buttonRight.attachClick(handleButtonRight);
  buttonLeft.attachDoubleClick(triggerServo);

  ultrasonic.setTimeout(40000UL);
  xTaskCreate(ultrasonicTask, "Ultrasonic task", 8192, NULL, 1, NULL);
  xTaskCreate(buttonTask, "Button task", 4096, NULL, 1, NULL);
  xTaskCreate(serialListenTask, "Serial listen task", 8192, NULL, 2, NULL);
}

void loop() {
  // FreeRTOS handle all tasks
}

void handleButtonLeft() {
  ServoTaskType task = LEFT;
  int result = xQueueSend(servoTasks, &task, 0);
  if (result != pdPASS) {
    Serial.println("Failed to send task to servo queue");
  } else {
    Serial.println("Added task LEFT successfully");
    printQueueSize(servoTasks);
  }
}

void handleButtonRight() {
  ServoTaskType task = RIGHT;
  int result = xQueueSend(servoTasks, &task, 0);
  if (result != pdPASS) {
    Serial.println("Failed to send task to servo queue");
  } else {
    Serial.println("Added task RIGHT successfully");
    printQueueSize(servoTasks);
  }
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
      break;
    case RIGHT:
      servoTurn(180);
      digitalWrite(LED_A, LOW);
      digitalWrite(LED_B, HIGH);
      Serial.println("Trigger task RIGHT from queue");
      break;
    default:
      Serial.println("Unknown command");
      digitalWrite(LED_A, LOW);
      digitalWrite(LED_B, LOW);
      break;
  }
  printQueueSize(servoTasks);
}

void servoTurn(int direction) {
  Serial.println("Write servo at " + String(direction) + " degrees");
  servo.write(direction);
}