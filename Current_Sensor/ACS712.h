#pragma once

class ACS712
{
  private:
  // Define the analog pin that the ACS712 current sensor module is connected to
  int8_t currentPin;                //4
  double vRef;                      // Reference voltage is 3.3V if not using AREF external reference         = 5.00
  double resConvert;                // ESP32 has 12-bit ADC, so 1024 possible values i.e 0 - 1023

  /*
  Sensitivity factor
  | Module            | current (A) | Sensitivity mV/A |  Sensitivity factor |
  | :---              | :---        | :---             |  :---               |
  | ACS712ELCTR-05B-T | ±5          | 185              |  0.185 V/A          |
  | ACS712ELCTR-20B-T | ±20         | 100              |  0.1 V/A            |
  | ACS712ELCTR-30B-T | ±30         | 66               |  0.066 V/A          |
  */
  double sensitivityFactor;

  double resADC;                    // Constants for A/D converter resolution
  double zeroPoint;                 // Zero point is half of Reference Voltage
  double vout;
  double current;

  public:
  ACS712();
  ACS712(int8_t=-1, double=3.3, double=4096, double=0.1);
  inline void begin() __attribute__((always_inline));
  inline double calculateCurrent() __attribute__((always_inline));
  inline void displayVoltage() __attribute__((always_inline));
  inline void printCurrent() __attribute__((always_inline));
};

ACS712::ACS712(int8_t currentPin, double vRef, double resConvert, double sensitivityFactor) :
currentPin(currentPin),
vRef(vRef),
resConvert(resConvert),
sensitivityFactor(sensitivityFactor)
{
  resADC = vRef/resConvert;   //0.00080566
  zeroPoint = vRef/2.0;       //1.65


  vout = 0.0;
  current = 0.0;
}

void ACS712::begin()
{
  pinMode(currentPin, INPUT);
}

double ACS712::calculateCurrent()
{
  // vout is read 1000 Times for precision
  for(int i = 0; i < 1000; i++) 
  {
    vout = (vout + (resADC * analogRead(currentPin)));   
    delay(1);
  }
  // Get vout in mv
  vout = vout /1000;
  // vout = resADC * analogRead(currentPin);
  
  // Convert vout into current using Scale Factor
  current = (vout - zeroPoint)/ sensitivityFactor;      
  if(current < 0) current = 0; 


  return current;                
}

void ACS712::printCurrent()
{
  // Serial.print("Vout = ");           
  // Serial.print(this->vout,2); 
  // Serial.print(" Volts");                            
  // Serial.print("\t Current = "); 
  // Serial.print(calculateCurrent(),2);
  // Serial.println(" Amps"); 

  Serial.println(analogRead(this->currentPin));
}