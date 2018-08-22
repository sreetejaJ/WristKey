void BLEMain() {
  onDisconnect();
  onConnected();
  onNewData();
  onCheckNeeded();
}

void onCheckNeeded() {
  if (checkNeeded && !deviceVerified) {
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
    pTxCharacteristic->setValue("ACKN:");
    pTxCharacteristic->notify();
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
          key = getCode();
        }
        pullData(findHash(input), password, username, convertString(key));
        Serial.println(username);
        Serial.println(password);

        sprintf(BLEOutbuf, "DATA:%s#%s", username.c_str(), password.c_str());
        Serial.println(BLEOutbuf);
        newData = true;
      } else {
        sprintf(BLEOutbuf, "ERRO:1");
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
      key = getCode();
    }
    storeData(convertString(key), convertString(dataIn[0]), convertString(dataIn[1]), convertString(dataIn[2]), NextAvailAddr());
  } else {
    Serial.println("Command not found");
  }
}

