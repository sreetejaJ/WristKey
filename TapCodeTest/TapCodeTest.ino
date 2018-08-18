#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

static byte button = 38;
static byte encA = 37;
static byte encB = 36;

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

char * characters[] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  u8g2.begin();
  pinMode(button, INPUT);
  pinMode(encA, INPUT);
  pinMode(encB, INPUT);
  u8g2.clearBuffer();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(getTapCode());
}

int getInput() {
  if (!digitalRead(button)) {
    while (!digitalRead(button)) {

    }
#ifdef DEBUG
    Serial.println("Button Pressed");
#endif
    return 10;
  }
  if (!digitalRead(encA) && digitalRead(encB)) {
    while (!digitalRead(encA) || !digitalRead(encB)) {
      delay(10);
    }
#ifdef DEBUG
    Serial.println("Encoder Decreased");
#endif
    return 1;
  } else if (!digitalRead(encB) && digitalRead(encA)) {
    while (!digitalRead(encA) || !digitalRead(encB)) {
      delay(10);
    }
#ifdef DEBUG
    Serial.println("Encoder Increased");
#endif
    return -1;
  }
  return 0;
}

String getTapCode(){
  int pos = 0;
  int lastPos = 0;
  String tapCode = "";
  byte tapPos = 0;
  while (true) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_17_tf);
    u8g2.setCursor(7, 15);
    u8g2.print("Enter Code");
    for (int i = 0; i < 6; i++) {
      u8g2.setCursor(40 + i * 15, 42);
      if(tapCode.length() > i){
        u8g2.print("*");
      }
      u8g2.drawFrame(38 + i * 15, 31, 12, 13);
    }
    u8g2.setFont(u8g2_font_logisoso28_tf);
    u8g2.setCursor(5, 55);
    u8g2.print(characters[pos]);
    u8g2.sendBuffer();
    while (pos == lastPos) {
      int val = getInput();
      if (val == 10) {
        tapCode += characters[pos];
        if(tapPos >= 5){
          u8g2.setCursor(40 + 5 * 15, 42);
          u8g2.print("*");
          delay(80);
          return tapCode;
        }
        tapPos++;
        break;
      }
      pos += val;
    }
    if (pos > 25) {
      pos = 0;
    }
    if (pos < 0) {
      pos = 25;
    }
    lastPos = pos;
  }
}

