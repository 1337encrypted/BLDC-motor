#include "OLEDFunctions.hpp"
#include "BUZZER.hpp"
#include "PC817A.hpp"
#include "ACS712.hpp"
#include "UART.hpp"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ACS712 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t currentPin1 = A1;              // 1
constexpr int8_t currentPin2 = A6;              // 6
constexpr double vRef = 5.00;                   // Reference voltage is 3.3V if not using AREF external reference
constexpr double resConvert = 1024;             // Arduino nano has 10-bit ADC, so 1024 possible values i.e 0 - 1024
constexpr double sensitivityFactor = 0.1;       // As its a 20Amp circuit

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PC817A ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t voltagePin1 = A0;
// constexpr int8_t voltagePin2 = A2;
constexpr double resADC = 0.0717; //0.0732

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ BUZZER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t buzzPin = 3;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MISCELLANEOUS VALUES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


double current1 = 0;
double current2 = 0;
double voltage1 = 0;


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Creation ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ACS712 ammeter1(currentPin1, vRef, resConvert, sensitivityFactor);
ACS712 ammeter2(currentPin2, vRef, resConvert, sensitivityFactor);
PC817A voltMeter1(voltagePin1, resADC);
// PC817A voltMeter2(voltagePin2, resADC);
buzzer buzz(buzzPin);
