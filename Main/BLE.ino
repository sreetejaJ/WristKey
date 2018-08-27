void BLEMain() {
  onDisconnect();
  onConnected();
  onNewData();
  onCheckNeeded();
}

void onCheckNeeded() {
  if (checkNeeded && !deviceVerified) {
    u8g2.setPowerSave(0);
    powerSaveTime = millis();
    if (Code()) {
      deviceVerified = true;
      checkNeeded = false;
      sprintf(BLEOutbuf, "VRFY:true");
      newData = true;
    }
  }
  if (millis() > verifyTime + timeout) {
    Serial.println("Verification Timed out");
    deviceVerified = false;
    checkNeeded = false;
    sprintf(BLEOutbuf, "ERRO:3");
    newData = true;
    verifyTime = millis();
    key = "";
  }
}

void onNewData() {
  if (newData) {
    pTxCharacteristic->setValue(BLEOutbuf);
    pTxCharacteristic->notify();
    memset(BLEOutbuf, 0, sizeof(BLEOutbuf));
    newData = false;
  }
}

void onConnected() {
  // do stuff here on connecting
  if (deviceConnected && !oldDeviceConnected && verified) {
    sprintf(BLEOutbuf, "ACKN:%d", map(NextAvailAddr(), 0, EEPROM_SIZE, 0, 100));
    newData = true;
    oldDeviceConnected = deviceConnected;
  }
}

void onDisconnect() {
  if (!deviceConnected && oldDeviceConnected) {
    delay(200); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
#ifdef DEBUG
    Serial.println("start advertising");
#endif
    oldDeviceConnected = deviceConnected;
    verified = false;
  }
}

void inputAction(String command, String input) {
  if (command == "RTRV:") {
    checkNeeded = true;
    if (deviceVerified) {
      Serial.println("Retrieving details");
      String username;
      String password;
      if (findHash(input) > 0) {
        if (key == "") {
          u8g2.setPowerSave(0);
          powerSaveTime = millis();
          key = getCode();
        }
        pullData(findHash(input), password, username, convertString(key));
        Serial.println(username);
        Serial.println(password);

        sprintf(BLEOutbuf, "DATA:%s#%s", username.c_str(), password.c_str());
        Serial.println(BLEOutbuf);
        newData = true;
      } else {
        sprintf(BLEOutbuf, "ERRO:%s", "1");
        newData = true;
        Serial.println("Hash not found");
      }
    }
  } else if (command == "ADDP:") {
    Serial.println("Adding");
    Serial.println(input);
    String dataIn[] = {"", "", ""};
    int count = 0;
    int i = 0;
    while (i < input.length()) {
      if (input.charAt(i) == '#') {
        count++;
        i++;
        if (count > 3) {
          Serial.println("data invalid");
        }
      }
      dataIn[count] += input[i];
      i++;
    }
    Serial.println(convertString(dataIn[0]));
    Serial.println(convertString(dataIn[1]));
    Serial.println(convertString(dataIn[2]));
    if (key == "") {
      u8g2.setPowerSave(0);
      powerSaveTime = millis();
      key = getCode();
    }
    storeData(convertString(key), convertString(dataIn[0]), convertString(dataIn[1]), convertString(dataIn[2]), NextAvailAddr());
    sprintf(BLEOutbuf, "UPMM:%d", map(NextAvailAddr(), 0, EEPROM_SIZE, 0, 100));
    newData = true;
  } else if (command == "DELT:") {
    if (findHash(input) > 0) {
      deleteData(findHash(input));
      sprintf(BLEOutbuf, "UPMM:%d", map(NextAvailAddr(), 0, EEPROM_SIZE, 0, 100));
      newData = true;
    } else {
      sprintf(BLEOutbuf, "ERRO:%s", "4");
      newData = true;
    }
  } else if (command == "GENP:") {
    if (deviceVerified) {
      Serial.println("Retrieving details");
      String password;
      for (int i = 0; i < 32; i++) {
        long randomNum1;
        long randomNum2;
        randomNum1 = random(26);
        password += characters[randomNum1];
      }
      sprintf(BLEOutbuf, "GENP:%s", password.c_str());
      Serial.println(BLEOutbuf);
      newData = true;
    } else {
      sprintf(BLEOutbuf, "ERRO:%s", "5");
      newData = true;
      Serial.println("Hash not found");
    }
  } else {
    Serial.println("Command not found");
  }
}


