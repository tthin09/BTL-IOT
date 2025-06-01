#ifndef SERVO_TASKS_H
#define SERVO_TASKS_H

#include <Arduino.h>
#include <ESP32Servo.h>

enum ServoTaskType {
    LEFT,
    RIGHT
};

extern Servo servo;
extern const int servoPin;
extern int currentServoDirection;

void servoTurn(int direction);
void addTaskLeft(QueueHandle_t& queue);
void addTaskRight(QueueHandle_t& queue);
void triggerServo(ServoTaskType task);

#endif // SERVO_TASKS_H