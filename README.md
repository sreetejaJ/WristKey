# WristKey


Repository containing all the test code used throughout to develop the WristKey wearable password manager. Final program is contained within the MainSketch folder.


Components:
-------------------------
  - 1 x Heltec ESP32 Development board with an OLED onboard
  - 1 x DS3231 Real Time Clock
  - 1 x Rotary Encoder
  
  
  
External libraries used:
--------------------------
  - Arduino Core for ESP32 - https://github.com/espressif/arduino-esp32
  - u8g2 display library - https://github.com/olikraus/u8g2
  - mBed TLS library - Included in Arduino Core
  - Rtc library - https://github.com/Makuna/Rtc
  
TODO:
--------------------------
  - Rotary encoder still glitches
  - Display frame rate needs improving
  - BLE should leverage standard BLE Service/Characteristics method
  - BLE library takes up a large program space
  
Pinout Reference:
--------------------------
![alt text](https://i.ebayimg.com/images/g/jXwAAOSwskFZgyPB/s-l1600.jpg)

Notes:
--------------------------
Save 12ish kb of program space by removing strings from BLEUtils (https://github.com/espressif/arduino-esp32/issues/1075)
