#include "OLEDFunctions.hpp"
#include "PC817A.hpp"
#include "ACS712.hpp"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ACS712 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t currentPin = 4;                // 4
constexpr double vRef = 5.00;                   // Reference voltage is 3.3V if not using AREF external reference
constexpr double resConvert = 1024;             // ESP32 has 12-bit ADC, so 4096 possible values i.e 0 - 4096
constexpr double sensitivityFactor = 0.1;       // As its a 20Amp circuit

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Creation ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ACS712 ammeter1(currentPin, vRef, resConvert, sensitivityFactor);

char currentArr[5] = {0};
char voltageArr[5] = {0};
// char powerArr[5] = {0};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PC817A ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t voltagePin = A1;
constexpr double vGain = 18.2;

PC817A voltMeter(voltagePin, vGain);