#pragma once
#include "BLDCPulseCalculator.hpp"

class motorSynchronization{
  private:
  BLDCPulseCalculator motor1, motor2;
  speed;

  public:
  inline motorSynchronization(const BLDCPulseCalculator=null, const BLDCPulseCalculator=null) __attribute__((always_inline));
  inline 

};

motorSynchronization::motorSynchronization(const BLDCPulseCalculator motor1, const BLDCPulseCalculator motor2) :
motor1(motor1),
motor2(motor2)
{}

