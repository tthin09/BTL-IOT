#include "servo.h"

void servoTurn(int direction) {
  Serial.println("Write servo at " + String(direction) + " degrees");
  servo.write(direction);
}

void addTaskLeft(QueueHandle_t& queue) {
  ServoTaskType task = LEFT;
  int result = xQueueSend(queue, &task, 0);
  if (result != pdPASS) {
    Serial.println("Failed to send task to servo queue");
  } else {
    Serial.println("Added task LEFT successfully");
  }
}

void addTaskRight(QueueHandle_t& queue) {
  ServoTaskType task = RIGHT;
  int result = xQueueSend(queue, &task, 0);
  if (result != pdPASS) {
    Serial.println("Failed to send task to servo queue");
  } else {
    Serial.println("Added task RIGHT successfully");
  }
}

void triggerServo(ServoTaskType task) {
  switch (task) {
    case LEFT:
      int left_degree = 180;
      servoTurn(left_degree);
      break;
    case RIGHT:
      int right_degree = 0;
      servoTurn(right_degree);
      break;
    default:
      Serial.println("Unknown task to trigger servo");
      break;
  }
}