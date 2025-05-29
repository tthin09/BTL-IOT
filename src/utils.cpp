#include "utils.h"

void printQueueSize(QueueHandle_t& queue) {
    Serial.println("Total item in queue: " + String(uxQueueMessagesWaiting(queue)));
}
