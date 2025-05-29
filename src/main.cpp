#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <ESP32Servo.h>
#include <Ultrasonic.h>
#include <OneButton.h>
#include "servo.h"

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

// Helper function for servo
void servoTurn(int direction);
void servoAddTask(ServoTaskType& task);
void triggerServo();

void ultrasonicTask(void *pvParameters) {
  while (1) {
    vTaskDelay(2000);
    double distance = ultrasonic.read(CM);
    Serial.println("Ultrasonic distance: " + String(distance) + " cm");
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
  xTaskCreate(ultrasonicTask, "Ultrasonic task", 8092, NULL, 1, NULL);
}

void loop() {
  buttonLeft.tick();
  buttonRight.tick();
  delay(10);
}

void handleButtonLeft() {
  ServoTaskType task = LEFT;
  int result = xQueueSend(servoTasks, &task, 0);
  if (result != pdPASS) {
    Serial.println("Failed to send task to servo queue");
  } else {
    Serial.println("Added task LEFT successfully");
  }
}

void handleButtonRight() {
  ServoTaskType task = RIGHT;
  int result = xQueueSend(servoTasks, &task, 0);
  if (result != pdPASS) {
    Serial.println("Failed to send task to servo queue");
  } else {
    Serial.println("Added task RIGHT successfully");
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
      break;
    case RIGHT:
      servoTurn(180);
      digitalWrite(LED_A, LOW);
      digitalWrite(LED_B, HIGH);
      break;
    default:
      Serial.println("Unknown command");
      digitalWrite(LED_A, LOW);
      digitalWrite(LED_B, LOW);
      break;
  }
}

void servoTurn(int direction) {
  servo.write(direction);
}