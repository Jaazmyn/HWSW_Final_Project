#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"  // Provides checkForBeat()

MAX30105 particleSensor;

const long WINDOW_DURATION = 10000; // 10-second data collection window
const long IR_THRESHOLD = 50000;    // IR value threshold to determine if a finger is detected

unsigned long windowStartTime;
int beatCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing MAX30105...");

  // Initialize sensor on I2C (400kHz)
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 not found. Please check wiring/power.");
    while (1);
  }
  Serial.println("Place your finger on the sensor with steady pressure.");

  // Configure sensor with default settings:
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);  // Low brightness for Red LED
  particleSensor.setPulseAmplitudeGreen(0);   // Turn off Green LED

  // Start a new data window
  windowStartTime = millis();
}

void loop() {
  long irValue = particleSensor.getIR();

  // Check if finger is detected by ensuring IR reading is above threshold
  if (irValue < IR_THRESHOLD) {
    Serial.println("No finger detected. Please place your finger on the sensor.");
    // Reset beat count and restart the window
    beatCount = 0;
    windowStartTime = millis();
    delay(1000); // Wait a bit before checking again
    return;
  }

  // If a beat is detected, increment beat count
  if (checkForBeat(irValue)) {
    beatCount++;
    Serial.print("Beat detected. Count: ");
    Serial.println(beatCount);
    // Optionally add a short delay to avoid counting the same beat multiple times.
    delay(300);
  }

  // When the 10-second window has elapsed, calculate and display BPM
  if (millis() - windowStartTime >= WINDOW_DURATION) {
    int bpm = beatCount * 6; // (10 sec window * 6 = 60 sec)
    Serial.print("Average BPM over 10 seconds: ");
    Serial.println(bpm);

    // Reset for the next window
    beatCount = 0;
    windowStartTime = millis();
  }
}