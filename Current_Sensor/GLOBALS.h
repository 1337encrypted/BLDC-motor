// #include "Display.h"
#include "ACS712.h"
#include "BLDCPulseCalculator.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ACS712 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t currentPin = 4;               // 4
constexpr double vRef = 3.30;                   // Reference voltage is 3.3V if not using AREF external reference
// constexpr double vRef = 5.00;                   // Reference voltage is 3.3V if not using AREF external reference
constexpr double resConvert = 4096;             // ESP32 has 12-bit ADC, so 1024 possible values i.e 0 - 1023 
constexpr double sensitivityFactor = 0.1;       // As its a 20Amp circuit

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ BLDCPulseCalculatior ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

constexpr int8_t wavePin1 = 2;
constexpr int8_t wavePin2 = 15;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Creation ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ACS712 ACS712Sensor(currentPin, vRef, resConvert, sensitivityFactor);
// BLDCPulseCalculator pulseCalculator1(wavePin1);
// BLDCPulseCalculator pulseCalculator2(wavePin2);