#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "SAMD51_DMX.h"
#include "SAMD51_E131.h"

#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5

#define CS 10
byte MAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress IP(10, 10, 2, 1);
IPAddress SUBNET(255, 255, 252, 0);

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

void setup() {
  Serial.begin(115200);
  
  Ethernet.init(CS);
  E131.begin(MAC, IP, SUBNET);

  DMX.begin(81);

  display.begin(0x3C, true); // Address 0x3C default
//  display.display();
//  delay(1000);
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setRotation(3);
  display.setCursor(0,0);
  display.clearDisplay();
  display.display();

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
}

bool parity = false;

int packets = 0;
bool debug = false;
bool serial = false;
bool pattern = true;

bool a = false;
bool b = false;
bool c = false;

void loop() {
  if (C()) {
    debug = !debug;
    if (!debug) {
      display.clearDisplay();
      display.display();
    }
  }

  if (B())
    serial = !serial;

  if (A())
    pattern = !pattern;

  if (pattern) {
    float t = micros() / 1000000.0;
    float hue = fmod(t / 16.0, 1.0);
    
    uint32_t rgbw = hsv2rgb(hue, 1.0, 1.0);
    uint8_t r = (rgbw >> 16) & 0xFF;
    uint8_t g = (rgbw >> 8) & 0xFF;
    uint8_t b = rgbw & 0xFF;
    
    for (int i = 0; i < 10; i++) {
      DMX.set(8 * i + 4, 255);
      DMX.set(8 * i + 5, r);
      DMX.set(8 * i + 6, g);
      DMX.set(8 * i + 7, b);
    }

    DMX.flush();
  }
  
  uint16_t ch = E131.parsePacket();
  if (ch && E131.universe == 1) {
    parity = !parity;
    digitalWrite(13, parity);
    
    if (serial) {
      Serial.print(ch);
      Serial.print(" ");
      for (int i = 1; i < ch; i++) {
        Serial.print(E131.data[i]);
        Serial.print(" ");
      }
      Serial.print("\n");
    }

    if (!pattern) {
      DMX.set(E131.data, ch);
      DMX.flush(ch + 1);
    }
    
    packets = E131.stats.num_packets;
  }

  if (debug) {
    display.clearDisplay();
    display.setCursor(0, 0);

    display.print("Link: ");
    auto link = Ethernet.linkStatus();
    switch (link) {
      case LinkON:
        display.println("UP");
        break;
      case LinkOFF:
        display.println("DOWN");
        break;
      case Unknown:
        display.println("UNK");
        break;
    }
  
    display.print("IP: ");
    if (link == LinkON)
      display.println(Ethernet.localIP());
    else
      display.println();

    display.print("Mask: ");
    if (link == LinkON)
      display.println(Ethernet.subnetMask());
    else
      display.println();
      
      
    display.print("Packets: ");
    display.println(packets);
    
    display.println();

    display.println();
    display.print("Pattern: ");
    display.println(pattern ? "Enabled" : "Disabled");
    
    display.print("Serial: ");
    display.println(serial ? "Enabled" : "Disabled");

    display.display();
  }
}


bool A() {
  bool _a = digitalRead(BUTTON_A);
  if (_a != a && _a == LOW) {
    a = _a;
    return true;
  } else {
    a = _a;
    return false;
  }
}

bool B() {
  bool _b = digitalRead(BUTTON_B);
  if (_b != b && _b == LOW) {
    b = _b;
    return true;
  } else {
    b = _b;
    return false;
  }
}

bool C() {
  bool _c = digitalRead(BUTTON_C);
  if (_c != c && _c == LOW) {
    c = _c;
    return true;
  } else {
    c = _c;
    return false;
  }
}

float fract(float x) { return x - int(x); }
float mix(float a, float b, float fr) { return a + (b - a) * fr; }
uint32_t hsv2rgb(float h, float s, float v) {
  float rr = v * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  float gg = v * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  float bb = v * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);

  byte r = (byte)(rr * 255);
  byte g = (byte)(gg * 255);
  byte b = (byte)(bb * 255);

  return (r << 16) | (g << 8) | b;
}
