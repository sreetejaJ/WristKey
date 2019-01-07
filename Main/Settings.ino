void settings() {
  while (true) {
    switch (menu("Select style", "Set timeout", "Reset Device", "Exit", 3)) {
      case 0:
        //selectFace();
        break;
      case 1:
        setTimeoutVal();
        break;
      case 2:
        resetDevice();
        break;
      case 3:
        return;
      default:
        break;
    }
  }
}

void sysInfo() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_17_tf);
  u8g2.setCursor(0, 16);
  u8g2.print("Chip ID:");
  u8g2.setCursor(0, 30);
  char buf[13];
  uint64_t chipid = ESP.getEfuseMac();
  sprintf(buf, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
  u8g2.print(buf);
  u8g2.sendBuffer();
  int val = 0;
  while (val != 10) {
   xQueueReceive(queue, &val, 0);
  }
  return;
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

void resetDevice() {
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
    if (no) {
      u8g2.drawBox(28, 62, 20, 4);
    } else {
      u8g2.drawBox(73, 62, 30, 4);
    }
    u8g2.sendBuffer();
    bool last = no;
    while (last == no) {
      int val = getInput();
      if (val == 10) {
        //Button Pressed
        if (no) {
          return;
        } else {
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_t0_17_tf);
          u8g2.setCursor(0, 15);
          u8g2.print("Deleting all");
          u8g2.setCursor(0, 30);
          u8g2.print("your data");
          u8g2.sendBuffer();
          delay(1000);
          for (int addr = PACKETS_START_ADDR; addr < EEPROM_SIZE; addr++) {
            EEPROM.write(addr, 0xFF);
          }
          EEPROM.commit();
          for (int i = 0; i < EEPROM_SIZE; i++) {
            Serial.print(EEPROM.read(i), HEX);
            Serial.print(" ");
            if ((i % 16) == 0) {
              Serial.println();
            }
          }
          ESP.restart();
        }
      }
      if (val != 0) {
        no = !no;
      }
    }
  }
}

void setTimeoutVal() {
  int lastTimeout = timeout;
  preferences.begin("settings", false);
  while (true) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_17_tf);
    u8g2.setCursor(0, 15);
    u8g2.print("Timeout:");
    u8g2.setCursor(10, 40);
    Serial.println("Setting timeout");
    u8g2.printf("< %d s >", lastTimeout / 1000);
    u8g2.setCursor(0, 60);
    u8g2.print("Press to exit");
    u8g2.sendBuffer();
    while (timeout == lastTimeout) {
      int val = getInput();
      if (val == 10) {
        return;
      } else if (val != 0) {
        timeout += val * 30;
        Serial.printf("%d\n", timeout);
      }
    }
    if (timeout < 0) {
      timeout = 600000;
    } if (timeout > 600000); {
      timeout = 30000;
    }
    lastTimeout = timeout;
  }
}

