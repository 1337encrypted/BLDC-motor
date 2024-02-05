//This module reads the voltage upto 60v max

#pragma once

class PC817A{
  private:
  int8_t voltagePin;
  double vGain, vout, resADC;

  public:
  inline PC817A(int8_t = -1, double vGain = -1) __attribute__((always_inline));
  inline void begin() __attribute__((always_inline));
  inline double readVoltage() __attribute__((always_inline));
  inline void printVoltage() __attribute__((always_inline));
};


PC817A::PC817A(int8_t voltagePin, double vGain) :
voltagePin(voltagePin),
vGain(vGain)
{
  vout = 0;
  resADC = 0.0732;
}

void PC817A::begin(){
  if(voltagePin > 0)  pinMode(this->voltagePin, INPUT);
}

double PC817A::readVoltage(){  
  // 1024 - analogRead(voltagePin)) * 0.00488 * vGain;
  // return 0.0732*analogRead(voltagePin);
  for(int i = 0; i < 100; i++) 
  {
    vout = (vout + (resADC * analogRead(voltagePin)));
    delayMicroseconds(100);
  }

  vout = vout /100;

  // Serial.println(analogRead(voltagePin));
  return vout;
}

void PC817A::printVoltage(){
  Serial.print("\t Voltage = ");           
  Serial.print(readVoltage());
  Serial.print(" Volts");         
}