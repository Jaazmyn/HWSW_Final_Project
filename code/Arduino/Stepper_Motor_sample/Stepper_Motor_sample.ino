#include <AccelStepper.h>

// Define stepper motor connections
#define IN1  0  // Motor Coil A
#define IN2  1  // Motor Coil B
#define IN3  2 // Motor Coil C
#define IN4  3 // Motor Coil D

// Define stepper type (4-wire bipolar motor)
AccelStepper stepper(AccelStepper::FULL4WIRE, IN1, IN2, IN3, IN4);

void setup() {
    stepper.setMaxSpeed(1000); // Set max speed (steps/second)
    stepper.setAcceleration(500); // Set acceleration (steps/second^2)
    Serial.begin(9600);
}

void loop() {
    Serial.println("Moving forward...");
    stepper.moveTo(500); // Move 500 steps forward
    while (stepper.distanceToGo() != 0) {
        stepper.run(); // Keep running until target reached
    }
    
    delay(1000);

    Serial.println("Moving backward...");
    stepper.moveTo(0); // Move back to start
    while (stepper.distanceToGo() != 0) {
        stepper.run();
    }

    delay(1000);
}