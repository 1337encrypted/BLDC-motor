#include "OLEDFunctions.hpp"
#include "BUZZER.hpp"
#include "PC817A.hpp"
#include "ACS712.hpp"
#include "UART.hpp"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ACS712 Current Sensors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t currentPin1 = A1;              // Motor 1 current sensor
constexpr int8_t currentPin2 = A6;              // Motor 2 current sensor
constexpr double vRef = 5.00;                   // Reference voltage (5V for Arduino Nano)
constexpr double resConvert = 1024;             // Arduino Nano has 10-bit ADC (0-1023)
constexpr double sensitivityFactor = 0.1;       // ACS712-20A: 100mV/A = 0.1V/A

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PC817A Voltage Sensor ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t voltagePin1 = A0;              // System voltage monitoring
// constexpr int8_t voltagePin2 = A2;
constexpr double resADC = 0.0717;               // Calibrated ADC resolution for voltage divider

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Buzzer Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t buzzPin = 3;                   // PWM pin for buzzer

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Global Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Current sensor readings
double current1 = 0;
double current2 = 0;
double voltage1 = 0;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Creation ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Sensor objects
ACS712 ammeter1(currentPin1, vRef, resConvert, sensitivityFactor);
ACS712 ammeter2(currentPin2, vRef, resConvert, sensitivityFactor);
PC817A voltMeter1(voltagePin1, resADC);

// Interface objects
buzzer buzz(buzzPin);

// --- New Robust Communication Protocol ---
constexpr uint8_t SOF_BYTE = 0xAA; // Start of Frame
constexpr uint8_t EOF_BYTE = 0x55; // End of Frame

#pragma pack(push, 1)
struct UARTPacket {
    uint8_t sof;
    MotorCharacteristicsData data;
    uint8_t checksum;
    uint8_t eof;
};
#pragma pack(pop)
// --- End of Protocol Definition ---

// Global data instance
MotorCharacteristicsData data;

// --- Updated UART Transmission Function ---
uint8_t calculateChecksum(const MotorCharacteristicsData& data_struct) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&data_struct);
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(data_struct); ++i) {
        checksum ^= bytes[i];
    }
    return checksum;
}

void sendData() {
    UARTPacket packet;
    packet.sof = SOF_BYTE;
    packet.data = data; // Copy global data into packet
    packet.checksum = calculateChecksum(packet.data);
    packet.eof = EOF_BYTE;

    // Send the entire packet over serial
    Serial.write(reinterpret_cast<uint8_t*>(&packet), sizeof(packet));
}

// Debug print function (memory optimized)
void motorCharacteristicsprintData() {
    Serial.print(F("Current1: "));
    Serial.print(data.current1);
    Serial.print(F(" A, Current2: "));
    Serial.print(data.current2);
    Serial.print(F(" A, Voltage: "));
    Serial.print(data.voltage1);
    Serial.println(F(" V"));
}

// OLED display update function (forward declaration)
void updateOLEDDisplay(float power1, float power2);