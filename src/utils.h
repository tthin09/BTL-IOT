#ifndef UTILS_H
#define UTILS_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <Arduino.h>
#include <Wire.h>

void printQueueSize(QueueHandle_t& queue);
void i2cScanner();

#endif // UTILS_H