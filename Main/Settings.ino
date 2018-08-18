void settings() {
  while (true) {
    switch (menu("Select style", "Reset Device", "Exit", "")) {
      case 0:
        selectFace();
        break;
      case 1:
        resetDevice();
        break;
      case 2:
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

void resetDevice(){
  bool no = true;
  while (true) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_17_tf);
    u8g2.setCursor(0, 15);
    u8g2.print("All saved data");
    u8g2.setCursor(0, 30);
    u8g2.print("will be lost");
    u8g2.setCursor(0, 45);
    u8g2.print("Continue?");
    u8g2.setCursor(75, 60);
    u8g2.print("Yes");
    u8g2.setCursor(30, 60);
    u8g2.print("No");
    if(no){
      u8g2.drawBox(28,62, 20, 4);
    }else{
      u8g2.drawBox(73,62, 30, 4);
    }
    u8g2.sendBuffer();
    bool last = no;
    while(last == no){
      int val = getInput();
      if(val == 10){
        //Button Pressed
        if(no){
          return;
        }else{
          ESP.restart();
        }
      }
      if(val != 0){
        no = !no;
      }
    }
  }
}

