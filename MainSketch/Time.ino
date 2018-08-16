void showTime(){
  u8g2.clearBuffer();
  RtcDateTime now = Rtc.GetDateTime();
  if(!clockface){
    ClockFace1(now);
  }else{
    ClockFace2(now);
  }
  if(millis() > timenow + updateRate){
    u8g2.sendBuffer();
    timenow = millis();
  }
  if (!digitalRead(button)) {
    while(!digitalRead(button)){
    //Wait for user to release
    }
    #ifdef SERIAL
      Serial.println("button pressed");
    #endif
    page = 1;
  }
  return;
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
