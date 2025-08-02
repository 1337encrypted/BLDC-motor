/***************************************************
 * Current Sensor Arduino Nano Project
 * 
 * Hardware:
 * - Arduino Nano
 * - 2x ACS712 Current Sensors (20A variant)  
 * - 1x PC817A Voltage Sensor
 * - SH1106 OLED Display (128x64, I2C)
 * - Buzzer for overcurrent alarm
 * 
 * Communication:
 * - UART to ESP32 @ 115200 baud
 * - Sends structured data: current1, current2, voltage1
 * 
 * Features:
 * - Real-time current and voltage monitoring
 * - OLED display with sensor readings
 * - Overcurrent protection with buzzer alarm
 * - UART data transmission to main controller
 ***************************************************/

#include <Arduino.h>
#include "GLOBALS.hpp"

void setup() {
    Serial.begin(115200);
    delay(100);

    // Initialize OLED display
    OLEDFunctions::begin();
    
    // Initialize current and voltage sensors
    ammeter1.begin();
    ammeter2.begin();
    voltMeter1.begin();
    
    // Initialize buzzer
    buzz.begin();
    buzz.initBuzzer();
    
    // Display startup message
    OLEDFunctions::clearBuffer();
    OLEDFunctions::oled.drawStr(15, 32, "Motor Control");
    OLEDFunctions::oled.drawStr(10, 45, "Current Monitor");
    OLEDFunctions::sendBuffer();
    delay(2000);
}

void loop() {
    // Read sensor data
    data.current1 = ammeter1.readCurrent();
    data.current2 = ammeter2.readCurrent();
    data.voltage1 = voltMeter1.readVoltage();
    
    // Calculate power values for display
    float power1 = data.current1 * data.voltage1;
    float power2 = data.current2 * data.voltage1;
    
    // Update OLED display
    updateOLEDDisplay(power1, power2);
    
    // Check for overcurrent condition (8A threshold)
    if (data.current1 > 8.0 || data.current2 > 8.0) {
        buzz.on();
    } else {
        buzz.off();
    }
    
    // Handle serial communication from ESP32
    while (Serial.available() > 0) {
        char receivedChar = Serial.read();
        
        if (receivedChar == 'S') { // ESP32 requests data
            sendData();
        }
        break;
    }
    
    delay(50); // 20Hz update rate
}

void updateOLEDDisplay(float power1, float power2) {
    // Convert float values to strings for display
    char current1Str[8], current2Str[8], voltage1Str[8];
    char power1Str[8], power2Str[8];
    
    dtostrf(data.current1, 4, 2, current1Str);
    dtostrf(data.current2, 4, 2, current2Str);  
    dtostrf(data.voltage1, 4, 1, voltage1Str);
    dtostrf(power1, 4, 1, power1Str);
    dtostrf(power2, 4, 1, power2Str);
    
    // Update OLED display
    OLEDFunctions::clearBuffer();
    OLEDFunctions::displayCurrent(current1Str, current2Str);
    OLEDFunctions::displayVoltage(voltage1Str);
    OLEDFunctions::displayPower(power1Str, power2Str);
    OLEDFunctions::sendBuffer();
}