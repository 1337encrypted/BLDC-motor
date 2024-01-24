/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ACS712 current sensor ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Define the analog pin that the ACS712 current sensor module is connected to
constexpr uint8_t currentPin = A1;

/*
  Constants for Scale Factor
  Use one that matches your version of ACS712

  Note the current sensitivity factor depends on the current rating of the module 
  | Module            | current (A) | Sensitivity mV/A |  Sensitivity factor |
  | :---              | :---        | :---             |  :---               |
  | ACS712ELCTR-05B-T | ±5          | 185              |  0.185 V/A          |
  | ACS712ELCTR-20B-T | ±20         | 100              |  0.1 V/A            |
  | ACS712ELCTR-30B-T | ±30         | 66               |  0.66 V/A           |
*/

//const double sensitivityFactor = 0.185; // 5A
const double sensitivityFactor = 0.1;     // 20A
//const double sensitivityFactor = 0.066; // 30A

// Variables for Measured Voltage and Calculated current
double vout = 0;
double current = 0;

// Constants for A/D converter resolution
// Arduino has 10-bit ADC, so 1024 possible values i.e 0 - 1023
// Reference voltage is 5V if not using AREF external reference
// Zero point is half of Reference Voltage

const double vRef = 5.00;
const double resConvert = 1024;
double resADC = vRef/resConvert;
double zeroPoint = vRef/2.0;

char buff[20];
char speedChar[5];



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ BLDC Speed ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;

constexpr uint8_t wavePin = 2;
unsigned long timeStamp = 0;
unsigned long nextTimeStamp = 0;
unsigned long timePeriod = 0;
unsigned long sumTime = 0;
unsigned long speed = 0;
uint8_t counter = 0;
unsigned long timePeriodValues[32] = {0};
unsigned long newTimePeriodValues[16] = {0};


enum class states : uint8_t
{
  IDLE,
  PRINT
};

volatile states status = states::IDLE;
