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
#include <Preferences.h>

#define DEBUG    // For use when testing, set 0 for final upload


bool checkHash(String value, int startAddr) {
  char attempt[value.length()];
  value.toCharArray(attempt, value.length() + 1);

  byte shaResult[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  const size_t payloadLength = strlen(attempt);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (unsigned char *) attempt, payloadLength);
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);


  for (int i = 0; i < sizeof(shaResult); i++) {
    //Serial.print(shaResult[i], HEX);
    //Serial.print(" ");
    if (EEPROM.read(i + startAddr) != shaResult[i]) {
      //Serial.println();
      return false;
    }
  }
  //Serial.println();
  return true;
}

//***************************RTC_START***************************
RtcDS3231<TwoWire> Rtc(Wire);
char* days[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
char* months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
char * endings[] = { "st", "nd", "rd", "th"};
char * characters[] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };
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

String key;
//***************************MEM_END*****************************

//***************************BLE_START***************************
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool newData = false;
char BLEOutbuf[1024];
bool verified = false;
bool checkNeeded = false;
bool deviceVerified = false;
unsigned long verifyTime = 0;
int timeout = 300000;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      verified = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 4) {
        String cmd;
        for (int i = 0; i < 5; i ++) {
          cmd += rxValue[i];
        }

        String dataIn;
        for (int i = 5; i < rxValue.length(); i++) {
          dataIn += rxValue[i];
          Serial.print(rxValue[i]);
        }
        Serial.println();
        if (cmd == "MPWD:") {
#ifdef DEBUG
          Serial.println(rxValue.length());
#endif
          if (checkHash(dataIn, 0x00)) {
            verified = true;
          } else {
            sprintf(BLEOutbuf, "ERRO:2");
            newData = true;
          }
        }
        if (verified) {
          inputAction(cmd, dataIn);
        }
      }
    }
};
//***************************BLE_END****************************

//***************************SETTINGS_START*********************
Preferences preferences;
//***************************SETTINGS_END***********************

//***************************HARDWARE_START*********************
static byte button = 38;
static byte encA = 37;
static byte encB = 36;

unsigned long powerSaveTime = 0;
int powerTimeout = 15000;
//***************************HARDWARE_STOP**********************
void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif
  //***************************OLED_SART***************************
  u8g2.begin();
  u8g2.clearBuffer();
  //***************************OLED_END****************************

  //***************************MEM_START***************************
  if (!EEPROM.begin(EEPROM_SIZE))
  {
#ifdef DEBUG
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

  //***************************RTC_START**************************
  Rtc.Begin();
  if (!Rtc.GetIsRunning())
  {
#ifdef DEBUG
    Serial.println("RTC was not actively running, starting now");
#endif
    Rtc.SetIsRunning(true);
  }
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  //***************************RTC_END****************************

  //***************************HARDWARE_START*********************
  pinMode(button, INPUT);
  pinMode(encA, INPUT);
  pinMode(encB, INPUT);
  powerSaveTime = millis();
  //***************************HARDWARE_STOP**********************

  //while(!Code());
  //Serial.println(getCode());
  String username;
  //storeData("abcdefghijklmnop", "www.facebook.com", "Mr.Robot", "TheZuccIsUnHackable", NextAvailAddr());
  //    Serial.println(findHash("www.facebook.com"));
  //    if(findHash("twitter") > 0){
  //      deleteData(findHash("twitter"));
  //    }
  //else{
  //      Serial.println("Hash not found");
  //    }
  showTime();
}

void loop() {
  switch (page) {
    case 0:
#ifdef DEBUG
      //Serial.println("Displaying Time");
#endif
      showTime();
      break;
    case 1:
#ifdef DEBUG
      Serial.println("Main Menu");
#endif
      switch (menu("Settings", "System Info", "Exit", "", 2)) {
        case 0:
          settings();
          break;
        case 1:
          sysInfo();
          break;
        case 2:
          page = 0;
          break;
        default:
          page = 0;
          break;
      }
      break;
    default:
      break;
  }
  BLEMain();
  if (didTimeOut()) {
    u8g2.setPowerSave(1);
    while (getInput() == 0) {
      BLEMain();
    }
    u8g2.setPowerSave(0);
    powerSaveTime = millis();
  }
}

int getInput() {
  if (!digitalRead(button)) {
    while (!digitalRead(button)) {

    }
#ifdef DEBUG
    Serial.println("Button Pressed");
#endif
    return 10;
  }
  if (!digitalRead(encA) && digitalRead(encB)) {
    while (!digitalRead(encA) || !digitalRead(encB)) {
      delay(10);
    }
#ifdef DEBUG
    Serial.println("Encoder Decreased");
#endif
    return 1;
  } else if (!digitalRead(encB) && digitalRead(encA)) {
    while (!digitalRead(encA) || !digitalRead(encB)) {
      delay(10);
    }
#ifdef DEBUG
    Serial.println("Encoder Increased");
#endif
    return -1;
  }
  return 0;
}
