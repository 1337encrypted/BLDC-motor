#include <Arduino.h>
#include "GLOBALS.hpp"

void setup() {
    Serial.begin(115200);
    delay(100);

    // Initialize components
    ammeter1.begin();
    ammeter2.begin();
    voltMeter1.begin();
    buzz.begin();
}

void loop() {
    // Handle serial communication
    while (Serial.available() > 0) {

      char receivedChar = Serial.read(); // Read a single byte
          
      // Read sensor data
      data.current1 = ammeter1.readCurrent();
      data.current2 = ammeter2.readCurrent();
      data.voltage1 = voltMeter1.readVoltage();

      // Perform actions based on sensor data
      if (data.current1 > 8.0 || data.current2 > 8.0)
          buzz.on();
      else
          buzz.off();

      // // Simulated data update for testing
      // data.current1 = 7.00;
      // data.current2 = 7.00;
      // data.voltage1 = 48.00;

      if (receivedChar == 'S') { // Check if the received character is 'S'
          sendData(); // Send data to ESP32
      }
  }
}
