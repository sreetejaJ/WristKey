#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include "EEPROM.h"
#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERIAL 1    // For use when testing, set 0 for final upload

//***************************RTC_START***************************
RtcDS3231<TwoWire> Rtc(Wire);
char* days[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
char* months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
char * endings[] = { "st", "nd", "rd", "th"};

//***************************RTC_END*****************************

//***************************OLED_START**************************
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

int page = 0;
unsigned long timenow = 0;
int updateRate = 100;
//***************************OLED_END****************************

//***************************MEM_START***************************
#define MASTER_PASS_HASH_ADDR 0x00
#define TAP_CODE_HASH_ADDR 0x20
#define PACKETS_START_ADDR 0x40
#define EEPROM_SIZE 4096

//***************************MEM_END*****************************

//***************************BLE_START***************************
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};
//***************************BLE_END****************************

//***************************SETTINGS_START*********************
bool clockface = false;
//***************************SETTINGS_END***********************

//***************************HARDWARE_START*********************
static byte button = 38;
static byte encA = 37;
static byte encB = 36;
//***************************HARDWARE_STOP**********************
void setup() {
  #ifdef SERIAL
    Serial.begin(115200);
  #endif
//***************************OLED_SART***************************
  u8g2.begin();
  u8g2.clearBuffer();
//***************************OLED_END****************************
  
//***************************MEM_START***************************
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    #ifdef SERIAL
      Serial.println("failed to initialise EEPROM");
    #endif
    u8g2.setFont(u8g2_font_t0_17_tf);
    u8g2.setCursor(0, 20);
    u8g2.print("Could not initialise EEPROM");
    u8g2.setCursor(0, 40);
    u8g2.print("Restarting device");
    u8g2.sendBuffer();
    delay(3000);
    ESP.restart();
  }
//***************************MEM_END*****************************

//***************************BLE_START***************************
  BLEDevice::init("wrsky");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
//***************************BLE_END****************************

//***************************RTC_START***************************
  Rtc.Begin();
  if (!Rtc.GetIsRunning())
  {
    #ifdef SERIAL
      Serial.println("RTC was not actively running, starting now");
    #endif
    Rtc.SetIsRunning(true);
  }
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
//***************************RTC_END*****************************

//***************************HARDWARE_START*********************
  pinMode(button, INPUT);
  pinMode(encA, INPUT);
  pinMode(encB, INPUT);
//***************************HARDWARE_STOP**********************

  showTime();
}

void loop() {
  switch(page){
    case 0:
      #ifdef SERIAL
        Serial.println("Displaying Time");
      #endif
      showTime();
      break;
    case 1:
      #ifdef SERIAL
        Serial.println("Settings Menu");
      #endif
      delay(1000);
      page = 0;
      break;
    default:
      break;
  }
}
