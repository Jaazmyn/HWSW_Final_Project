#include <MAX30105.h>
#include <heartRate.h>
#include <spo2_algorithm.h>
#include <Wire.h>

#define MAX_SAMPLES 100    // Number of samples to capture for each calculation

MAX30105 particleSensor; // MAX30102 sensor object

// Buffers for raw sensor readings:
uint32_t irBuffer[MAX_SAMPLES];   // Infrared LED readings
uint32_t redBuffer[MAX_SAMPLES];  // Red LED readings

// Variables to hold calculated results:
int32_t spo2;           // Calculated SpO2 percentage
int8_t validSPO2;       // 1 if SpO2 reading is valid, 0 otherwise
int32_t heartRate;      // Calculated heart rate in BPM
int8_t validHeartRate;  // 1 if heart rate reading is valid, 0 otherwise

// ----------------------------------------------------------
// Function: maxim_heart_rate_and_oxygen_saturation
// This simplified algorithm uses peak detection on the IR signal
// to determine heart rate and computes an approximate SpO2
// value based on the ratio of red to IR readings.
// ----------------------------------------------------------
void maxim_heart_rate_and_oxygen_saturation(uint32_t *irBuffer, int bufferLength, uint32_t *redBuffer,
                                            int32_t *spo2, int8_t *validSPO2,
                                            int32_t *heartRate, int8_t *validHeartRate) {
  int i;
  unsigned long sumIR = 0, sumRed = 0;
  
  // Calculate the mean of the IR and Red signals
  for (i = 0; i < bufferLength; i++) {
    sumIR += irBuffer[i];
    sumRed += redBuffer[i];
  }
  unsigned long meanIR = sumIR / bufferLength;
  unsigned long meanRed = sumRed / bufferLength;

  // --- Heart Rate Calculation ---
  // We'll detect peaks in the IR buffer. A peak is defined as a value higher than its neighbors and above the mean.
  int peakCount = 0;
  int peakIndices[MAX_SAMPLES] = {0};

  for (i = 1; i < bufferLength - 1; i++) {
    if (irBuffer[i] > irBuffer[i - 1] && irBuffer[i] > irBuffer[i + 1] && irBuffer[i] > meanIR) {
      peakIndices[peakCount++] = i;
    }
  }

  // Compute heart rate if we detected at least two peaks.
  if (peakCount >= 2) {
    float totalIntervals = 0;
    for (i = 1; i < peakCount; i++) {
      totalIntervals += (peakIndices[i] - peakIndices[i - 1]);
    }
    float avgInterval = totalIntervals / (peakCount - 1);
    // Assume a sampling rate of ~25 Hz. BPM = (25 / avgInterval) * 60.
    *heartRate = (int32_t)((25.0 / avgInterval) * 60.0);
    *validHeartRate = 1;
  } else {
    *heartRate = -1;
    *validHeartRate = 0;
  }

  // --- SpO2 Calculation ---
  // Compute an average ratio between red and IR signals relative to their means.
  float ratioSum = 0;
  int validCount = 0;
  for (i = 0; i < bufferLength; i++) {
    if (irBuffer[i] > 0 && redBuffer[i] > 0) {
      float ratio = ((float)redBuffer[i] / meanRed) / ((float)irBuffer[i] / meanIR);
      ratioSum += ratio;
      validCount++;
    }
  }
  if (validCount > 0) {
    float ratioAvg = ratioSum / validCount;
    // Use an approximate formula: SpO2 = 110 - 25 * (ratioAvg)
    float spo2Calc = 110.0 - 25.0 * ratioAvg;
    // Clamp the value between 85% and 100%
    if (spo2Calc > 100.0) spo2Calc = 100.0;
    if (spo2Calc < 85.0)  spo2Calc = 85.0;
    *spo2 = (int32_t)spo2Calc;
    *validSPO2 = 1;
  } else {
    *spo2 = -1;
    *validSPO2 = 0;
  }
}

// ----------------------------------------------------------
// Setup and Loop
// ----------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Initializing MAX30102...");

  Wire.begin();

  if (!particleSensor.begin()) {
    Serial.println("MAX30102 not found. Check wiring.");
    while (1);  // Halt execution if sensor is not detected
  }

  Serial.println("MAX30102 detected successfully.");

  // Configure the sensor:
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0xFF);  // Set Red LED to maximum brightness
  particleSensor.setPulseAmplitudeIR(0xFF);   // Set IR LED to maximum brightness
}
void loop() {
  Serial.println("Collecting data...");

  // Collect MAX_SAMPLES samples for analysis.
  for (int i = 0; i < MAX_SAMPLES; i++) {
    // Wait until new data is available.
    while (!particleSensor.check());
    redBuffer[i] = particleSensor.getRed();  // Read Red LED data
    irBuffer[i] = particleSensor.getIR();      // Read IR LED data
  }

  // Calculate heart rate and SpO2 from the collected samples.
  maxim_heart_rate_and_oxygen_saturation(irBuffer, MAX_SAMPLES, redBuffer,
                                         &spo2, &validSPO2, &heartRate, &validHeartRate);

  // Display the results on the Serial Monitor.
  Serial.print("Heart Rate: ");
  if (validHeartRate)
    Serial.print(heartRate);
  else
    Serial.print("Invalid");

  Serial.print(" BPM | SpO2: ");
  if (validSPO2)
    Serial.print(spo2);
  else
    Serial.print("Invalid");
  Serial.println(" %");

  delay(1000);  // Wait 2 seconds before the next measurement cycle
}