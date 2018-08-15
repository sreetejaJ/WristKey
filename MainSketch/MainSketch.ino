#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include "EEPROM.h"
#include "mbedtls/aes.h"
#include "mbedtls/md.h"

#define SERIAL 1    // For use when testing, set 0 for final upload

//***************************RTC_START***************************
RtcDS3231<TwoWire> Rtc(Wire);
char* days[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
char* months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
char * endings[] = { "st", "nd", "rd", "th"};
//***************************RTC_END*****************************


//***************************OLED_START**************************
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
//***************************OLED_END****************************



//***************************MEM_START***************************
#define MASTER_PASS_HASH_ADDR 0x00
#define TAP_CODE_HASH_ADDR 0x20
#define PACKETS_START_ADDR 0x40
#define EEPROM_SIZE 4096

//***************************MEM_END*****************************
void setup() {
  #ifdef SERIAL
    Serial.begin(115200);
  #endif
  
//***************************MEM_START***************************
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    #ifdef SERIAL
      Serial.println("failed to initialise EEPROM");
    #endif
  }
  
//***************************MEM_END*****************************

//***************************OLED_SART***************************
  u8g2.begin();
  u8g2.clearBuffer();
//***************************OLED_END****************************

}

void loop() {
  // put your main code here, to run repeatedly:

}
