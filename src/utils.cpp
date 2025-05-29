#include "utils.h"

void printQueueSize(QueueHandle_t& queue) {
    Serial.println("Total item in queue: " + String(uxQueueMessagesWaiting(queue)));
}

void i2cScanner() {
  byte error, address;
  int nDevices;

  Serial.println("Scanning I2C bus...");
  Serial.println("-------------------");

  nDevices = 0;
  for (address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmission to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknow error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found.");
  } else {
    Serial.println("Done scanning.");
  }
  Serial.println("-------------------");
}