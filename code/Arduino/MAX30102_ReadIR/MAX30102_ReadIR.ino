#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor; // MAX30102 sensor object
TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(1);

void setup() {
  I2Cone.begin(SDA_1, SCL_1, freq1);
  I2Ctwo.begin(SDA_2, SCL_2, freq2); 
}

setup(){
  Wire.begin(); //uses default SDA and SCL and 100000HZ freq
  Wire1.begin(SDA_2, SCL_2, freq);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing MAX30102...");

    Wire.begin();

    if (!particleSensor.begin()) { 
        Serial.println("MAX30102 not found. Check wiring.");
        while (1);
    }

    Serial.println("MAX30102 detected successfully.");

    // Configure sensor
    particleSensor.setup(); 
    particleSensor.setPulseAmplitudeRed(0xFF); // Adjust LED brightness
    particleSensor.setPulseAmplitudeIR(0xFF);
}

void loop() {
    long irValue = particleSensor.getIR(); // Read IR light

    if (irValue < 50000) { 
        Serial.println("No finger detected.");
    } else {
        Serial.print("IR Value: ");
        Serial.println(irValue);
    }

    delay(500);
}