/*
 * UART Communication Header for Arduino Nano
 * 
 * This file contains the data structure and functions for 
 * communicating current and voltage sensor data to ESP32
 * via UART at 115200 baud rate.
 * 
 * The data is sent as a binary struct when ESP32 sends 'S' command.
 */

#pragma once

// Forward declaration of class
class buzzer;

// Motor characteristics data structure (matches ESP32 receiver)
typedef struct {
    float current1;    // Motor 1 current in Amps
    float current2;    // Motor 2 current in Amps  
    float voltage1;    // System voltage in Volts
} MotorCharacteristicsData;

// Forward declarations from GLOBALS.hpp
extern MotorCharacteristicsData data;
extern buzzer buzz;

// Functions declarations only - implementations in separate functions
void sendData();
void motorCharacteristicsprintData();

// Global variables for WiFi information
char receivedIPAddress[16] = "No IP Set";
bool ipAddressReceived = false;

// WiFi mode information
char wifiMode = 'U';  // U=Unknown, A=AP Mode (Field), S=Station Mode (Home/Office)
bool wifiModeReceived = false;
char wifiModeIP[16] = "";

// Display mode: 0=Normal (Power), 1=IP Only, 2=WiFi Mode + IP
uint8_t displayMode = 0;

// Function to handle IP address reception (memory optimized)
void handleIPAddressReception() {
    static char ipBuffer[16];
    static uint8_t bufferIndex = 0;
    
    while (Serial.available() > 0) {
        char inChar = Serial.read();
        
        if (inChar == '\n' || inChar == '\r') {
            // End of IP address string
            if (bufferIndex > 0) {
                ipBuffer[bufferIndex] = '\0'; // Null terminate
                strcpy(receivedIPAddress, ipBuffer);
                ipAddressReceived = true;
                
                // Log received IP address (if debugging enabled)
                Serial.print(F("IP: "));
                Serial.println(receivedIPAddress);
                
                // Reset buffer for next reception
                bufferIndex = 0;
                return;
            }
        } else if (bufferIndex < 15) { // Prevent buffer overflow
            ipBuffer[bufferIndex++] = inChar;
        }
    }
}

// Function to handle WiFi mode reception
void handleWiFiModeReception() {
    static char modeBuffer[20];
    static uint8_t bufferIndex = 0;
    
    while (Serial.available() > 0) {
        char inChar = Serial.read();
        
        if (inChar == '\n' || inChar == '\r') {
            // End of mode command string
            if (bufferIndex > 0) {
                modeBuffer[bufferIndex] = '\0'; // Null terminate
                
                // Parse mode command: First char is mode (A/S), rest is IP
                if (bufferIndex > 1) {
                    wifiMode = modeBuffer[0]; // A or S
                    strcpy(wifiModeIP, &modeBuffer[1]); // IP address
                    wifiModeReceived = true;
                    
                    // Set display mode based on received info
                    displayMode = 2; // WiFi Mode + IP display
                    
                    Serial.print(F("WiFi Mode: "));
                    Serial.print(wifiMode == 'A' ? F("Field (AP)") : F("Home/Office (STA)"));
                    Serial.print(F(", IP: "));
                    Serial.println(wifiModeIP);
                }
                
                // Reset buffer for next reception
                bufferIndex = 0;
                return;
            }
        } else if (bufferIndex < 19) { // Prevent buffer overflow
            modeBuffer[bufferIndex++] = inChar;
        }
    }
}

// Function to handle incoming UART commands
void handleUARTCommands() {
    if (Serial.available() > 0) {
        char receivedChar = Serial.peek(); // Look at first character without consuming it
        
        if (receivedChar == 'I') {
            // IP address command - consume the 'I' and handle IP reception
            Serial.read(); // Consume the 'I'
            handleIPAddressReception();
            displayMode = 1; // IP only display
            return;
        }
        
        if (receivedChar == 'M') {
            // WiFi Mode command - consume the 'M' and handle mode reception
            Serial.read(); // Consume the 'M'
            handleWiFiModeReception();
            return;
        }
        
        // Handle single character commands
        receivedChar = Serial.read();
        switch (receivedChar) {
            case 'S':  // Send data command from ESP32
                sendData();
                break;
                
            case 'D':  // Debug print command
                motorCharacteristicsprintData(); 
                break;
                
            case 'R':  // Reset/restart command
                buzz.initBuzzer();  // Confirmation beep
                break;
                
            default:
                // Ignore unknown commands
                break;
        }
    }
}