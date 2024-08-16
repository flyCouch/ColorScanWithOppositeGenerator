#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_GFX_Library.h>
#include <Adafruit_SSD1306.h>
#include <tone.h>

int speakerPin 9;

// Define pins for OLED
#define OLED_SDA 4
#define OLED_SCL 5

// Create instances of the libraries
/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

uint16_t r, g, b, c, colorTemp, lux;

const int ScanPin = 2;
bool Scan;

byte newR, newG, newB;

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground */

/* Initialise with default values (int time = 2.4ms, gain = 1x) */
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();

/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);

void RGBtoHSV(byte r, byte g, byte b, float &h, float &s, float &v);
void HSVtoRGB(float h, float s, float v, byte &r, byte &g, byte &b);

void setup() {
  pinMode(speakerPin, OUTPUT);

  // Initialize the sensor and display
  tcs.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address might need adjustment
  pinMode(ScanPin, INPUT);
  Serial.begin(9600);
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
}

void loop() {
  Scan = digitalRead(ScanPin);
  if (Scan == HIGH); {
    ScanColor();

    // Calculate complementary color
    byte cr = 255 - r;
    byte cg = 255 - g;
    byte cb = 255 - b;

    // Convert complementary color to HSV for manipulation
    float h, s, v;
    RGBtoHSV(cr, cg, cb, h, s, v);

    // Maximize brightness and saturation
    s = 1.0; // Maximum saturation
    v = 1.0; // Maximum value (brightness)

    // Convert back to RGB
    //byte newR, newG, newB;
    HSVtoRGB(h, s, v, newR, newG, newB);

    // ... output the results
    Serial.print("Original RGB: ");
    Serial.print(r);
    Serial.print(", ");
    Serial.print(g);
    Serial.print(", ");
    Serial.println(b);

    Serial.print("Brighter Complementary RGB: ");
    Serial.print(newR);
    Serial.print(", ");
    Serial.print(newG);
    Serial.print(", ");
    Serial.println(newB);
    Serial.println(" ");
    Scan = LOW;
  }
  // Read color data from TCS34725
  uint16_t clear, red, green, blue;
  tcs.getRGB(&red, &green, &blue, &clear);

  tone(speakerPin, 432); // Generate a 432 Hz tone on speakerPin
  delay(200); // Adjust delay as needed
  noTone(speakerPin); // Stop the tone

  // Display RGB values on the OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("R:");
  display.print(red);
  display.setCursor(0, 10);
  display.print("G:");
  display.print(green);
  display.setCursor(0, 20);
  display.print("B:");
  display.print(blue);
  display.display();

  // Display RGB values on the OLED

  display.setCursor(3, 0);
  display.print("R:");
  display.print(newR);
  display.setCursor(3, 10);
  display.print("G:");
  display.print(newG);
  display.setCursor(3, 20);
  display.print("B:");
  display.print(newB);
  display.display();

}

// RGB to HSV conversion
void RGBtoHSV(byte r, byte g, byte b, float & h, float & s, float & v) {
  float r_f = r / 255.0;
  float g_f = g / 255.0;
  float b_f = b / 255.0;
  float cmax = max(r_f, max(g_f, b_f));
  float cmin = min(r_f, min(g_f, b_f));
  v = cmax;
  float delta = cmax - cmin;
  if (delta == 0) {
    h = 0;
    s = 0;
  } else {
    s = delta / cmax;
    if (cmax == r_f) {
      h = 60 * (fmod((g_f - b_f) / delta, 6));
    } else if (cmax == g_f) {
      h = 60 * ((b_f - r_f) / delta + 2);
    } else if (cmax == b_f) {
      h = 60 * ((r_f - g_f) / delta + 4);
    }
    if (h < 0) {
      h += 360;
    }
  }
}

// HSV to RGB conversion
void HSVtoRGB(float h, float s, float v, byte & r, byte & g, byte & b) {
  float c = v * s;
  float h_prime = h / 60.0;
  float x = c * (1 - abs(fmod(h_prime, 2) - 1));
  float m = v - c;

  int hi = (int)h_prime;
  switch (hi) {
    case 0:
      r = (c + m) * 255;
      g = (x + m) * 255;
      b = (m) * 255;
      break;
    case 1:
      r = (x + m) * 255;
      g = (c + m) * 255;
      b = (m) * 255;
      break;
    case 2:
      r = (m) * 255;
      g = (c + m) * 255;
      b = (x + m) * 255;
      break;
    case 3:
      r = (m) * 255;
      g = (x + m) * 255;
      b = (c + m) * 255;
      break;
    case 4:
      r = (x + m) * 255;
      g = (m) * 255;
      b = (c + m) * 255;
      break;
    case 5:
      r = (c + m) * 255;
      g = (m) * 255;
      b = (x + m) * 255;
      break;
  }
}
void ScanColor() {
  //uint16_t r, g, b, c, colorTemp, lux;
  tcs.getRawData(&r, &g, &b, &c);
  delay(154); // Delay for one old integ. time period (to finish old reading)
  delay(615); // Delay for one new integ. time period (to allow new reading)
  // colorTemp = tcs.calculateColorTemperature(r, g, b);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  lux = tcs.calculateLux(r, g, b);

  Serial.println("Integ. time: 614.4ms ");
  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");
}

