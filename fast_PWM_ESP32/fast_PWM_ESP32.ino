const int motor1 = 12;
// const int motor2 = 26;

// Define pin numbers
// constexpr uint8_t potentiometerPin = 34; // Analog pin to read potentiometer value
// constexpr uint8_t psudoPowerPin = 14;

// LEDC settings
const int freq = 100000;           // PWM frequency in Hz
const int resolution = 8;        // PWM resolution (8-bit)
const uint8_t dutyCycle = 127;

void setup() {
  // Configure LEDC PWM
  ledcAttach(motor1, freq, resolution);
  // ledcAttach(motor2, freq, resolution);

  pinMode(psudoPowerPin, OUTPUT);
  // pinMode(potentiometerPin, INPUT);

  // Set up Serial Monitor
  Serial.begin(115200);
}

void loop() {
  // Read potentiometer value
  // int potValue = analogRead(potentiometerPin);

  // Map potentiometer value to LEDC duty cycle range (0-255)
  // int dutyCycle = map(potValue, 0, 4095, 0, 255);

  // Write duty cycle to LED
  ledcWrite(motor1, dutyCycle);
  // ledcWrite(motor2, dutyCycle);

  // Print potentiometer value and duty cycle
  // Serial.print("Potentiometer Value: ");
  // Serial.print(potValue);
  Serial.print(", Duty Cycle: ");
  Serial.println(dutyCycle);

  // Add delay if needed
  delay(100);
}
