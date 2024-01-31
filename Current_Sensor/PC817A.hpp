//This module reads the voltage upto 60v max

#pragma once

class PC817A{
  private:
  int8_t voltagePin;
  double vGain;

  public:
  inline PC817A(int8_t = -1, double vGain = -1) __attribute__((always_inline));
  inline void begin() __attribute__((always_inline));
  inline uint16_t readVoltage() __attribute__((always_inline));
  inline void printVoltage() __attribute__((always_inline));
};


PC817A::PC817A(int8_t voltagePin, double vGain) :
voltagePin(voltagePin),
vGain(vGain)
{}

void PC817A::begin(){
  if(voltagePin > 0)  pinMode(this->voltagePin, INPUT);
}

uint16_t PC817A::readVoltage(){
  return analogRead(voltagePin)*vGain;
}

void PC817A::printVoltage(){
  Serial.print(readVoltage());
}