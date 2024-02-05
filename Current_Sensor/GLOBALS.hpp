#include "OLEDFunctions.hpp"
#include "BUZZER.hpp"
#include "PC817A.hpp"
#include "ACS712.hpp"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ACS712 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t currentPin1 = A1;              // 1
constexpr int8_t currentPin2 = A6;              // 6
constexpr double vRef = 5.00;                   // Reference voltage is 3.3V if not using AREF external reference
constexpr double resConvert = 1024;             // ESP32 has 12-bit ADC, so 4096 possible values i.e 0 - 4096
constexpr double sensitivityFactor = 0.1;       // As its a 20Amp circuit

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PC817A ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t voltagePin1 = A0;
constexpr int8_t voltagePin2 = A2;
constexpr double vGain = 18.2;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ BUZZER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t buzzPin = 3;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MISCELLANEOUS VALUES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

char currentArr1[5] = {0};
char currentArr2[5] = {0};
char voltageArr1[5] = {0};
// char voltageArr2[5] = {0};
char powerArr1[5] = {0};
char powerArr2[5] = {0};

double current1 = 0;
double current2 = 0;
double voltage1 = 0;
double voltage2 = 0;
double power1 = 0;
double power2 = 0;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Creation ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ACS712 ammeter1(currentPin1, vRef, resConvert, sensitivityFactor);
ACS712 ammeter2(currentPin2, vRef, resConvert, sensitivityFactor);
PC817A voltMeter1(voltagePin1, vGain);
PC817A voltMeter2(voltagePin2, vGain);
buzzer buzz(buzzPin);
