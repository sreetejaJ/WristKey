#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "EEPROM.h"


U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

#define EEPROM_SIZE 4096
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(EEPROM.length());
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
  Serial.println(EEPROM.length());
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB18_tr);
  String username = "";
  String password = "";

  pullData(0x21, password, username);

  Serial.print("\n\n\n");
  Serial.println(username);
  Serial.println(password);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void pullData(unsigned int addr, String &password, String &username){
  int dataSize;
   for(int i = 0; i < EEPROM_SIZE; i++){
    if(EEPROM.read(i) == NULL){
      dataSize = EEPROM.read(i + 1);
      addr = i + 2;
      break;
    }
  }
  Serial.println("Size of data is: ");
  Serial.print(dataSize);
  Serial.println();
  Serial.println("The website hash is: ");
  for(int i = 0; i < 32; i++){
    Serial.print(EEPROM.read(addr + i), HEX);
    Serial.print(" ");
  }
  addr += 32;
  Serial.println();
  Serial.println("The encrypted password length is: ");
  int Len = EEPROM.read(addr);
  Serial.print(Len);
  Serial.println();
  addr += 1;
  for(int i = 0; i < Len; i++){
    Serial.print(EEPROM.read(addr + i), HEX);
    password += String(EEPROM.read(addr + i));
    Serial.print(" ");
  }
  addr += 32;
  Serial.println();
  Serial.println("The encrypted username length is: ");
  Len = EEPROM.read(addr);
  Serial.print(Len);
  Serial.println();
  addr += 1;
  for(int i = 0; i < Len; i++){
    Serial.print(EEPROM.read(addr + i), HEX);
    username += String(EEPROM.read(addr + i));
    Serial.print(" ");
  }
}

