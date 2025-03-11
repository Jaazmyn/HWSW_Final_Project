#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h" // Library for heart rate & SpO2 calculations

MAX30105 particleSensor; // MAX30102 sensor object

// Buffer to store IR and Red LED values
#define MAX_SAMPLES 100 // Number of samples for calculation
uint32_t irBuffer[MAX_SAMPLES];  // Infrared LED readings
uint32_t redBuffer[MAX_SAMPLES]; // Red LED readings

int32_t spo2;           // SpO2 percentage
int8_t validSPO2;       // Validity check for SpO2
int32_t heartRate;      // Heart rate in BPM
int8_t validHeartRate;  // Validity check for heart rate

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
    particleSensor.setPulseAmplitudeRed(0xFF); // Max brightness
    particleSensor.setPulseAmplitudeIR(0xFF);
}

void loop() {
    Serial.println("Collecting data...");

    // Collect MAX_SAMPLES samples for SpO2 & Heart Rate calculations
    for (int i = 0; i < MAX_SAMPLES; i++) {
        while (!particleSensor.check()); // Wait for new data
        redBuffer[i] = particleSensor.getRed();  // Get Red LED value
        irBuffer[i] = particleSensor.getIR();    // Get IR LED value
    }

    // Calculate Heart Rate and SpO2
    maxim_heart_rate_and_oxygen_saturation(irBuffer, MAX_SAMPLES, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    // Display results
    Serial.print("Heart Rate: ");
    if (validHeartRate) Serial.print(heartRate);
    else Serial.print("Invalid");

    Serial.print(" BPM | SpO2: ");
    if (validSPO2) Serial.print(spo2);
    else Serial.print("Invalid");

    Serial.println(" %");

    delay(2000); // Wait before next reading
}