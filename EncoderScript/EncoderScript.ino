/*
  DigitalReadSerial

  Reads a digital input on pin 2, prints the result to the Serial Monitor

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/DigitalReadSerial
*/
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
// digital pin 2 has a pushbutton attached to it. Give it a name:
int encA = 36;
int encB = 37;
int button = 38;
int aLastState;
int bLastState;

int aState;
int bState;

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);


int counter = 0;
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  // make the pushbutton's pin an input:
  pinMode(encA, INPUT);
  pinMode(encB, INPUT);
  pinMode(button, INPUT);

  aLastState = digitalRead(encA);
  bLastState = digitalRead(encB);
  
  u8g2.begin();
  u8g2.clearBuffer();
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input pin:
  aState = digitalRead(encA);
  bState = digitalRead(encB);
  if (aLastState != aState) {
    if (aState != bState) {
        counter++;
    }else{
      counter--;
    }
  }else if (bLastState != bState) {
    if (aState != bState) {
        counter--;
    }else{
      counter++;
    }
  }
  
  aLastState = aState;
  bLastState = bState;
  
  Serial.print(aState);
  Serial.print("\t");
  Serial.print(bState);
  Serial.print("\t");
  //Serial.print(digitalRead(button));
  Serial.print("\t");
  //Serial.print(counter);
  Serial.print("\n");
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_18_tf);
  u8g2.setCursor(0, 30);
  u8g2.print(counter);
  u8g2.sendBuffer();
  delay(10);        // delay in between reads for stability
}


