#include <Arduino.h>

constexpr uint8_t wavePin=25;
unsigned long timeStamp=0;
unsigned long nextTimeStamp=0;
unsigned long timePeriod=0;


void ARDUINO_ISR_ATTR isr() {
  timeStamp = millis();
  timePeriod = timeStamp - nextTimeStamp;
  nextTimeStamp = timeStamp;
}

void setup() {
  Serial.begin(115200);
  attachInterrupt(wavePin, isr, RISING);
}

void loop() {
  Serial.println("Time Period" + timePeriod);
}
