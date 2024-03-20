typedef struct {
    float current1;
    float current2;
    float voltage1;
} MotorCharacteristicsData;

// Create an instance of the structure
MotorCharacteristicsData data;

void sendData() {
    // Serialize the struct data into a buffer
    uint8_t buffer[sizeof(data)];
    memcpy(buffer, &data, sizeof(data));

    // Send the serialized data over serial
    Serial.write(buffer, sizeof(buffer));

    // // // Print the length of the data
    // Serial.print("Length of data: ");
    // Serial.println(sizeof(buffer));

    // Serial.print("Length of data Structure : ");
    // Serial.println(sizeof(MotorCharacteristicsData));

    // // Print the content of the buffer
    // Serial.print("Buffer content: ");
    // for (int i = 0; i < sizeof(buffer); i++) {
    //     Serial.print(buffer[i], BIN);
    //     Serial.print(" ");
    // }
    // Serial.println();
}


void motorCharacteristicsprintData() {
  Serial.print(data.current1);
  Serial.print(" , ");
  Serial.print(data.current2);
  Serial.print(" , ");
  Serial.println(data.voltage1);
}



