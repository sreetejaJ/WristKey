#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <RtcDS3231.h>

//***************************RTC_START***************************
RtcDS3231<TwoWire> Rtc(Wire);
char* days[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
char* months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
char * endings[] = { "st", "nd", "rd", "th"};
//***************************RTC_END*****************************


//***************************OLED_START**************************
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
//***************************OLED_END****************************




void setup() {
  // put your setup code here, to run once:

//***************************OLED_SART***************************
  u8g2.begin();
  u8g2.clearBuffer();
//***************************OLED_END****************************

}

void loop() {
  // put your main code here, to run repeatedly:

}
