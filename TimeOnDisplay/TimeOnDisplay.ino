#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <RtcDS3231.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

RtcDS3231<TwoWire> Rtc(Wire);

char* days[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
char* months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
char * endings[] = { "st", "nd", "rd", "th"};

bool newPress = false;
int button = 38;
int counter = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(button, INPUT);

  
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  u8g2.begin();
  u8g2.clearBuffer();
}

void loop() {
  // put your main code here, to run repeatedly:
  u8g2.clearBuffer();
  RtcDateTime now = Rtc.GetDateTime();
  printDateTime(now);
  u8g2.sendBuffer();
  delay(1000);
  if (!digitalRead(button)) {
    counter++;
    Serial.println("button pressed");
  }
  //****************DISPLAY***************
}



#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  //Serial.print(datestring);
  Serial.println(counter);
  if (newPress) {
    counter++;
    newPress = false;
  }
  if ((counter % 2) == 0) {
    ClockFace1(dt);
  } else {
    ClockFace2(dt);
  }
}

void ClockFace1(const RtcDateTime& dt) {
  char timeStr[6];
  u8g2.setFont(u8g2_font_logisoso42_tf);
  sprintf(timeStr, "%02u:%02u", dt.Hour(), dt.Minute());
  u8g2.setCursor(0, 50);
  u8g2.print(timeStr);
  int len = map(dt.Second(), 0, 59, 0, 128);
  u8g2.drawBox(0, 60, len, 4);
}

void ClockFace2(const RtcDateTime& dt) {
  u8g2.setFont(u8g2_font_logisoso28_tf);
  char format[3];
  sprintf(format, "%02u", dt.Hour());
  u8g2.setCursor(0, 29);
  u8g2.print(format);

  sprintf(format, "%02u", dt.Minute());
  u8g2.setCursor(0, 63);
  u8g2.print(format);

  u8g2.setFont(u8g2_font_t0_17_tf);

  char fmt[5];
  int width = u8g2.getStrWidth(days[dt.DayOfWeek() - 1]);
  u8g2.setCursor(128 - width, 16);
  u8g2.print(days[dt.DayOfWeek() - 1]);


  int index = getEnding(dt.Day());
  sprintf(fmt, "%02u%s", dt.Day(), endings[index]);
  width = u8g2.getStrWidth(fmt);
  u8g2.setCursor(128 - width, 32);
  u8g2.print(fmt);

  width = u8g2.getStrWidth(months[dt.Month() - 1]);
  u8g2.setCursor(128 - width, 45);
  u8g2.print(months[dt.Month() - 1]);


  sprintf(fmt, "%04u", dt.Year());
  width = u8g2.getStrWidth(fmt);
  u8g2.setCursor(128 - width, 63);
  u8g2.print(fmt);
}

int getEnding(int day) {
  if (day == 1 || day == 21 || day == 31) {
    return 0;
  } else if (day == 2 || day == 22) {
    return 1;
  } else if (day == 3 || day == 23) {
    return 2;
  } else {
    return 3;
  }
}


