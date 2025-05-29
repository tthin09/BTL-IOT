#ifndef UTILS_H
#define UTILS_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <Arduino.h>

void printQueueSize(QueueHandle_t& queue);

#endif // UTILS_H