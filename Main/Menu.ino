int menu(String value1, String value2, String value3, String value4, int def) {
  int pos = 0;
  int lastPos = 0;
  bool first = true;
  int num = 1;
  if (value3 != "") {
    num++;
  }
  if (value4 != "") {
    num++;
  }
  powerSaveTime = millis();
  while (true) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_17_tf);
    u8g2.setCursor(10, 16);
    u8g2.print(value1);
    u8g2.setCursor(10, 32);
    u8g2.print(value2);
    u8g2.setCursor(10, 48);
    u8g2.print(value3);
    u8g2.setCursor(10, 64);
    u8g2.print(value4);
    //Serial.println(pos);
    u8g2.drawBox(0, pos * 16 + 4, 7, 13);
    u8g2.sendBuffer();
    //int val;
    while (pos == lastPos) {
      int val = getInput();
      if (val == 10) {
        return pos;
      }
      pos += val;
    }
    if (pos > num) {
      pos = 0;
    }
    if (pos < 0) {
      pos = num;
    }
    lastPos = pos;
  }
}

