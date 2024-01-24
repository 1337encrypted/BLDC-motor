#include "GLOBALS.h"
#include "Display.h"

void IRAM_ATTR calculateValues() 
{

  portENTER_CRITICAL_ISR(&synch);

  timeStamp = millis();
  timePeriodValues[counter++] = timeStamp - nextTimeStamp;
  nextTimeStamp = timeStamp;


  if((counter % 2) == 0 && counter != 0)
  {
    newTimePeriodValues[counter / 2 - 1] = timePeriodValues[counter - 1] + timePeriodValues[counter - 2];
  }

  if (counter == 32) 
  {
    counter = 0;
    status = states::PRINT;
  }

  portEXIT_CRITICAL_ISR(&synch);
}

void setup()
{ 
  Serial.begin(9600);

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  oledBegin();

  pinMode(wavePin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(wavePin), calculateValues, CHANGE);
  pinMode(currentPin, INPUT);
}

void loop()
{
  displayCurrent();    

  switch (status) 
  {
    case states::PRINT:
    for (int i = 0; i < 16; i++) 
    {
      sumTime += newTimePeriodValues[i];
    }
    timePeriod = (unsigned long)(sumTime / 16.0);
    speed = (unsigned long)(3750 / timePeriod); // (1000 * 60) / (time period * 16)

    // Serial.print(timePeriod, );

    // Serial.print("\nMotor Speed: ");
    Serial.print(speed);
    Serial.print(" ; ");

    ltoa(speed, speedChar, 3); 
    writeRPM(speedChar);

    
    sumTime = 0;
    
    status = states::IDLE;
    break;


    case states::IDLE:
    break;
  }

}


void displayCurrent()
{
  // vout is read 1000 Times for precision
  for(int i = 0; i < 500; i++) 
  {
    vout = (vout + (resADC * analogRead(currentPin)));   
    delay(1);
  }
  
  // Get vout in mv
  vout = vout /500;
  // vout = resADC * analogRead(currentPin);
  
  // Convert vout into current using Scale Factor
  current = (vout - zeroPoint)/ sensitivityFactor;      
  if(current < 0) current = 0;             

  // Serial.print("Vout = ");           
  // Serial.print(vout,2); 
  // Serial.print(" Volts");                            
  // Serial.print("\t Current = ");                  
  // Serial.print(current,2);
  // Serial.println(" Amps");  

  dtostrf(current, 5, 2, buff);
  
  writeCurrent(buff);      
}

