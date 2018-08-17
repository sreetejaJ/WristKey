void settings() {
  while (true) {
    switch (menu("Select style", "Exit", "", "")) {
      case 0:
        selectFace();
        break;
      case 1:
        return;
      default:
        break;
    }
  }
}

void sysInfo() {

}

void selectFace() {
  int pos = 0;
  int lastPos = 0;
  bool first = true;
  preferences.begin("settings", false);
  while (true) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_17_tf);
    u8g2.setCursor(10, 16);
    u8g2.print("Face 1");
    u8g2.setCursor(10, 32);
    u8g2.print("Face 2");
    u8g2.setCursor(10, 48);
    u8g2.drawBox(0, pos * 16 + 4, 7, 13);
    unsigned int selected = preferences.getUInt("ClockFace", 0);
    switch (selected) {
      case 0:
        u8g2.setCursor(110, 16);
        break;
      case 1:
        u8g2.setCursor(110, 32);
        break;
      default:
        break;
    }
    u8g2.print("<");
    u8g2.sendBuffer();
    while (pos == lastPos) {
      int val = getInput();
      if (val == 10) {
        preferences.putUInt("ClockFace", pos);
        preferences.end();
        return;
      }
      pos += val;
    }
    if (pos > 1) {
      pos = 0;
    }
    if (pos < 0) {
      pos = 1;
    }
    lastPos = pos;
  }
}

