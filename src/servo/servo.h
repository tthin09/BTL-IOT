#ifndef SERVO_TASKS_H
#define SERVO_TASKS_H

#include <Arduino.h>
#include <ESP32Servo.h>

enum ServoTaskType {
    LEFT,
    RIGHT
};

Servo servo;
const int servoPin = 27;
int currentServoDirection = 0;

void servoTurn(int direction);
void addTaskLeft(QueueHandle_t& queue);
void addTaskRight(QueueHandle_t& queue);
void triggerServo(ServoTaskType task);

#endif