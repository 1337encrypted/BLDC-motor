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
    OLEDFunctions::oled.firstPage();
    do {
        OLEDFunctions::oled.drawStr(25, 32, "BLDC");
        OLEDFunctions::oled.drawStr(15, 45, "Monitor");
    } while (OLEDFunctions::oled.nextPage());
    delay(2000);
    
    // Test OLED with sample data
    Serial.println(F("Testing OLED with sample data..."));
    OLEDFunctions::oled.firstPage();
    do {
        OLEDFunctions::displayEnhanced("1.25", "2.34", "12.0", "15.0", "28.1", "192.168.1.100", 'S', 3);
    } while (OLEDFunctions::oled.nextPage());
    delay(3000);
    
    Serial.println(F("Starting current sensor monitoring..."));
}

void loop() {
    // Read sensor data
    data.current1 = ammeter1.readCurrent();
    data.current2 = ammeter2.readCurrent();
    data.voltage1 = voltMeter1.readVoltage();
    
    // Calculate power values for display
    float power1 = data.current1 * data.voltage1;
    float power2 = data.current2 * data.voltage1;
    
    // Debug output every 1 second (20 cycles at 20Hz)
    static uint8_t debugCounter = 0;
    if (++debugCounter >= 20) {
        debugCounter = 0;
        Serial.print(F("DEBUG - I1: "));
        Serial.print(data.current1, 3);
        Serial.print(F("A, I2: "));
        Serial.print(data.current2, 3);
        Serial.print(F("A, V1: "));
        Serial.print(data.voltage1, 2);
        Serial.print(F("V, P1: "));
        Serial.print(power1, 2);
        Serial.print(F("W, P2: "));
        Serial.print(power2, 2);
        Serial.println(F("W"));
    }
    
    // Update OLED display
    updateOLEDDisplay(power1, power2);
    
    // Check for overcurrent condition (8A threshold)
    if (data.current1 > 8.0 || data.current2 > 8.0) {
        buzz.on();
        Serial.println(F("OVERCURRENT DETECTED!"));
    } else {
        buzz.off();
    }
    
    // Handle serial communication from ESP32
    handleUARTCommands();
    
    delay(50); // 20Hz update rate
}

void updateOLEDDisplay(float power1, float power2) {
    // Convert float values to strings for display with proper formatting
    char current1Str[8], current2Str[8], voltage1Str[8];
    char power1Str[8], power2Str[8];
    
    // Ensure proper string formatting with consistent width
    dtostrf(data.current1, 5, 2, current1Str);  // Format: " X.XX"
    dtostrf(data.current2, 5, 2, current2Str);  // Format: " X.XX"
    dtostrf(data.voltage1, 5, 1, voltage1Str);  // Format: " XX.X"
    dtostrf(power1, 5, 1, power1Str);           // Format: " XX.X"
    dtostrf(power2, 5, 1, power2Str);           // Format: " XX.X"
    
    // Remove leading spaces for cleaner display
    String temp1 = String(current1Str);
    temp1.trim();
    temp1.toCharArray(current1Str, 8);
    
    String temp2 = String(current2Str);
    temp2.trim();
    temp2.toCharArray(current2Str, 8);
    
    String temp3 = String(voltage1Str);
    temp3.trim();
    temp3.toCharArray(voltage1Str, 8);
    
    String temp4 = String(power1Str);
    temp4.trim();
    temp4.toCharArray(power1Str, 8);
    
    String temp5 = String(power2Str);
    temp5.trim();
    temp5.toCharArray(power2Str, 8);
    
    // Get WiFi strength and mode info
    int displayWiFiStrength = wifiStrengthReceived ? wifiStrength : 0;
    char displayMode = wifiModeReceived ? wifiMode : 'U'; // U for unknown
    char* displayIP = wifiModeReceived ? wifiModeIP : (ipAddressReceived ? receivedIPAddress : "");
    
    // Debug output for display strings
    static uint8_t displayDebugCounter = 0;
    if (++displayDebugCounter >= 40) { // Every 2 seconds
        displayDebugCounter = 0;
        Serial.print(F("DISPLAY - Current1: '"));
        Serial.print(current1Str);
        Serial.print(F("', Current2: '"));
        Serial.print(current2Str);
        Serial.print(F("', Voltage: '"));
        Serial.print(voltage1Str);
        Serial.println(F("'"));
    }
    
    // Update OLED display using enhanced layout
    OLEDFunctions::oled.firstPage();
    do {
        OLEDFunctions::displayEnhanced(current1Str, current2Str, voltage1Str, 
                                      power1Str, power2Str, displayIP, 
                                      displayMode, displayWiFiStrength);
    } while (OLEDFunctions::oled.nextPage());
}