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

// UART data structure and functions
typedef struct {
    float current1;
    float current2;
    float voltage1;
} MotorCharacteristicsData;

// Global data instance
MotorCharacteristicsData data;

// UART transmission function
void sendData() {
    // Serialize the struct data into a buffer
    uint8_t buffer[sizeof(data)];
    memcpy(buffer, &data, sizeof(data));

    // Send the serialized data over serial
    Serial.write(buffer, sizeof(buffer));
}

// Debug print function
void motorCharacteristicsprintData() {
    Serial.print("Current1: ");
    Serial.print(data.current1);
    Serial.print(" A, Current2: ");
    Serial.print(data.current2);
    Serial.print(" A, Voltage: ");
    Serial.print(data.voltage1);
    Serial.println(" V");
}

// OLED display update function (forward declaration)
void updateOLEDDisplay(float power1, float power2);