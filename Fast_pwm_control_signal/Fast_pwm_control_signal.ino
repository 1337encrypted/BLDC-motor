#include <TimerOne.h>

constexpr uint8_t pwmPin = 9; // Choose a PWM-capable pin (e.g., D9)
// Settings
static const uint8_t buf_len = 20;

char c;
char buf[buf_len];
uint8_t idx = 0;
int dutyCycle = 0;

void setup() {
  Serial.begin(9600);
  // Set the PWM pin as an output
  pinMode(pwmPin, OUTPUT);
  
  // Set up Timer1 with a frequency of 100 kHz
  Timer1.initialize(10); // Timer period in microseconds (1/100,000 Hz = 10 microseconds)
  Timer1.pwm(pwmPin, 0);
}

void loop() {
  // Read characters from serial
  while (Serial.available() > 0) {
    c = Serial.read();

    // Update dutyCycle variable if we get a newline character
    if (c == '\n') {

      dutyCycle = atoi(buf);
      
      if(dutyCycle > 1000)
        dutyCycle = 1000;

      Serial.println(dutyCycle);
      Timer1.pwm(pwmPin, dutyCycle); // PWM duty cycle (0 to 1023, where 0 is 0% and 1023 is 100%)
      idx = 0;
    } else {
      // Only append if index is not over message limit
      if (idx < buf_len - 1) {
        buf[idx] = c;
        idx++;
      }
    }
  }
}
