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

// Motor characteristics data structure (matches ESP32 receiver)
typedef struct {
    float current1;    // Motor 1 current in Amps
    float current2;    // Motor 2 current in Amps  
    float voltage1;    // System voltage in Volts
} MotorCharacteristicsData;

// Function to send sensor data to ESP32
void sendData() {
    // Serialize the struct data into a buffer
    uint8_t buffer[sizeof(MotorCharacteristicsData)];
    memcpy(buffer, &data, sizeof(data));

    // Send the serialized data over UART
    Serial.write(buffer, sizeof(buffer));
}

// Debug function to print data to serial monitor
void motorCharacteristicsprintData() {
    Serial.print("I1:");
    Serial.print(data.current1, 2);
    Serial.print("A I2:");
    Serial.print(data.current2, 2); 
    Serial.print("A V:");
    Serial.print(data.voltage1, 1);
    Serial.println("V");
}

// Function to handle incoming UART commands
void handleUARTCommands() {
    while (Serial.available() > 0) {
        char receivedChar = Serial.read();
        
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
        
        break; // Process one command per loop iteration
    }
}