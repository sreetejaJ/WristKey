void BLEMain() {
  onDisconnect();
  onConnected();
}

void onConnected() {
  // do stuff here on connecting
  if (deviceConnected && !oldDeviceConnected && verified) {
    pTxCharacteristic->setValue("ACKN:");
    pTxCharacteristic->notify();
    oldDeviceConnected = deviceConnected;
  }
}

void onDisconnect(){
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

