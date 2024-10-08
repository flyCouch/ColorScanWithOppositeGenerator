#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

int speakerPin = 9;
int OLED_RESET = -1;

// Define pins for OLED
#define OLED_SDA 4
#define OLED_SCL 5

// Create instances of the libraries
/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

uint16_t r, g, b, c, colorTemp, lux;

int ScanPin = 2;
bool Scan = LOW;

byte newR, newG, newB;
void RGBtoHSV(byte r, byte g, byte b, float &h, float &s, float &v);
void HSVtoRGB(float h, float s, float v, byte &r, byte &g, byte &b);

void setup() {
  pinMode(speakerPin, OUTPUT);
  pinMode(ScanPin, INPUT);
  Serial.begin(9600);

  // Initialize the sensor and display
  tcs.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address might need adjustment
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
}

void loop() {
  Scan = digitalRead(2);
  if (Scan == HIGH) {
    Serial.println(Scan);
    Scan = LOW;
    ScanColor();
    // ... output the results
    Serial.print("Original RGB: ");
    Serial.print(byte (r));
    Serial.print(", ");
    Serial.print(byte (g));
    Serial.print(", ");
    Serial.println(byte (b));

    // Display RGB values on the OLED
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(10, 0);
    display.print("Actual");
    display.setCursor(10, 20);
    display.print("R:");
    display.print(byte (r));
    display.setCursor(10, 35);
    display.print("G:");
    display.print(byte (g));
    display.setCursor(10, 50);
    display.print("B:");
    display.print(byte (b));

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

    Serial.print("Brighter Complementary RGB: ");
    Serial.print(newR);
    Serial.print(", ");
    Serial.print(newG);
    Serial.print(", ");
    Serial.println(newB);
    Serial.println(" ");

    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(70, 0);
    display.print("Laser");
    display.setCursor(70, 20);
    display.print("R:");
    display.print(newR);
    display.setCursor(70, 35);
    display.print("G:");
    display.print(newG);
    display.setCursor(70, 50);
    display.print("B:");
    display.print(newB);
    display.display();

    for (int i = 0; i < 51; i++) {
      digitalWrite(speakerPin, HIGH);
      delay(3);
      digitalWrite(speakerPin, LOW);
      delay(3);
    }
  }
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
  tcs.getRawData(&r, &g, &b, &c);
  delay(154); // Delay for one old integ. time period (to finish old reading)
  delay(615); // Delay for one new integ. time period (to allow new reading)
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  //colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  lux = tcs.calculateLux(r, g, b);
}
