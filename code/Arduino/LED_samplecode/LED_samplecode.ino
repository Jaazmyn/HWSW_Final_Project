#include <Adafruit_NeoPixel.h>

// Define the LED strip parameters
#define LED_PIN    8    // Pin connected to the LED strip
#define NUM_LEDS   20   // Number of LEDs in the strip

// Initialize the NeoPixel library
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    strip.begin();            // Initialize the LED strip
    strip.show();             // Turn off all LEDs initially
    strip.setBrightness(50);  // Adjust brightness (0-255)
}

void loop() {
    rainbowCycle(10); // Run a rainbow effect with 10ms delay
}

// Function to display a rainbow effect
void rainbowCycle(int wait) {
    for (int j = 0; j < 256; j++) {
        for (int i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel((i + j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

// Color wheel function for rainbow effect
uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}