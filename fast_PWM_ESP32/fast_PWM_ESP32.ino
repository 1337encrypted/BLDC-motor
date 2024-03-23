const int motor1 = 12;

// LEDC settings
const int freq = 20000;           // PWM frequency in Hz
const int resolution = 8;        // PWM resolution (8-bit)
const uint8_t dutyCycle = 127;

void setup() {
  // Configure LEDC PWM
  ledcAttach(motor1, freq, resolution);
}

void loop() {
  ledcWrite(motor1, dutyCycle);
}
