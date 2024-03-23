#include <TimerOne.h>

constexpr uint8_t pwmPin = 10; // Choose a PWM-capable pin (e.g., D9)
// Settings
// static const uint8_t buf_len = 20;

// char receivedChar;         
// char buf[buf_len];
// uint8_t idx = 0;
// int dutyCycle = 0;

void setup() {
  // Serial.begin(9600);
  // Set the PWM pin as an output
  pinMode(pwmPin, OUTPUT);
  
  // Set up Timer1 with a frequency of 100 kHz
  Timer1.initialize(50); // Set frequency to 10kHz (10us period)
  Timer1.pwm(pwmPin, 512); // 50% duty cycle (512/1023)
}

// void clearBuffer()
// {
//   for (int i = 0; i < buf_len; i++) 
//   {
//     buf[i] = 0;
//   }
// }

void loop() 
{
  // // Read characters from serial
  // while (Serial.available() > 0) 
  // {
  //   receivedChar = Serial.read();

  //   // Update dutyCycle variable if we get a newline character
  //   if (receivedChar == '\n') {

  //     dutyCycle = atoi(buf);

  //     Serial.println(dutyCycle);
  //     Timer1.pwm(pwmPin, dutyCycle); // PWM duty cycle (0 to 1023, where 0 is 0% and 1023 is 100%)
  //     clearBuffer();
  //     idx = 0;
  //   } 
  //   else
  //   {
  //     // Only append if index is not over message limit
  //     if (idx < buf_len - 1) 
  //     {
  //       buf[idx] = receivedChar;
  //       idx++;
  //     }
  //   }
  // }
}
