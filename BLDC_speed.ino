#include <Arduino.h>

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

states status = states::IDLE;

void ARDUINO_ISR_ATTR isr() 
{
  timeStamp = millis();
  timePeriod = timeStamp - nextTimeStamp;
  nextTimeStamp = timeStamp;

  if(timePeriod != 0)
  {
    timePeriodValues[counter++] = timePeriod;
  }

  if((counter % 2) == 0 && counter != 0)
  {
    newTimePeriodValues[(counter/2)-1] = timePeriodValues[counter-1] + timePeriodValues[counter-2];
  }

  if (counter == 32) 
  {
    counter = 0;
    status = states::PRINT;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(wavePin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(wavePin), isr, CHANGE);
}

void loop() {

  switch (status) 
  {
    case states::PRINT:
    for (int i = 0; i < 16; i++) 
    {
      sumTime += newTimePeriodValues[i];

    }
    timePeriod = (unsigned long)(sumTime / 16.0);
    speed = (unsigned long)(3750 / timePeriod); // (1000 * 60) / (time period * 16)

    Serial.print(timePeriod);
    Serial.print(",");
    Serial.println(speed);
    
    sumTime = 0;
    
    status = states::IDLE;
    break;


    case states::IDLE:
    break;
  }


  // vTaskDelay(100 / portTICK_PERIOD_MS);
}
