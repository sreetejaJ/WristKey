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


bool checkHash(String value, int startAddr);
void inputAction(String command, String input);

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

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
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
char BLEOutbuf[2048];
bool verified = false;
bool checkNeeded = false;
bool deviceVerified = false;
unsigned long verifyTime = 0;
int timeout = 300000;
unsigned long connectTime = 0;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define connectTimeout 600000

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
const byte button = 38;
const byte encA = 37;
const byte encB = 36;

unsigned long powerSaveTime = 0;
int powerTimeout = 15000;
volatile bool A_set = false;
volatile bool B_set = false;
volatile bool button_set = true;
volatile unsigned long debounceTime = millis();
QueueHandle_t queue;

#define DEBOUNCE_TIMEOUT 80

void IRAM_ATTR isr_encA(){
  int out = 0;
  A_set = digitalRead(encA) == LOW;
  // and adjust counter + if A leads B
  out += (A_set != B_set) ? +1 : -1;
  if(millis() - debounceTime > DEBOUNCE_TIMEOUT){ 
    xQueueSendFromISR(queue, &out, NULL);
    debounceTime = millis();
  }
}

void IRAM_ATTR isr_encB(){
  int out = 0;
  B_set = digitalRead(encB) == LOW;
  out += (A_set == B_set) ? +1 : -1;
  if(millis() - debounceTime > DEBOUNCE_TIMEOUT){
    xQueueSendFromISR(queue, &out, NULL);
    debounceTime = millis();
  }
}

void IRAM_ATTR isr_button(){
    int send = 10;
    if(millis() - debounceTime > DEBOUNCE_TIMEOUT){
      bool in = digitalRead(button);
      if(!in){
        button_set = false;
      }else if(!button_set){
        xQueueSendFromISR(queue, &send, NULL);
        button_set = true;
      }
      debounceTime = millis();
    }
}
//***************************HARDWARE_STOP**********************

//***************************GAMES_START************************
#define BALL_SIZE 6
#define PAD_HEIGHT 20
#define PAD_WIDTH 5
#define SPEED 10
//***************************GAMES_STOP************************


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
  queue = xQueueCreate(10, sizeof(signed int));
  if(queue == NULL){
    Serial.println("Error creating the queue");
  }
  attachInterrupt(digitalPinToInterrupt(button), isr_button, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encA), isr_encA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), isr_encB, CHANGE);
  powerSaveTime = millis();
  //***************************HARDWARE_STOP**********************

  //***************************PREFERENCES_START******************
  preferences.begin("settings", false);
  if (preferences.getUInt("timeout", 0) != 0) {
    timeout = preferences.getUInt("timeout", 0);                  // Timeout preference
  }
  //***************************PREFERENCES_STOP*******************

  //logo();
  showTime();
}

void loop() {
  BLEMain();
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
      switch (menu("System Info", "Settings", "Exit", "", 2)) {
        case 0:
          sysInfo();
          break;
        case 1:
          settings();
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
  int val;
  if(xQueueReceive(queue, &val, 0)){
   Serial.println("Data from queue");
   if(val == 10){
    page = 1;
   }
  }
  //  if (didTimeOut()) {
  //    u8g2.setPowerSave(1);
  //    while (getInput() == 0) {
  //      BLEMain();
  //    }
  //    u8g2.setPowerSave(0);
  //    powerSaveTime = millis();
  //  }
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

#define WristKeyLogoSmall_width 64
#define WristKeyLogoSmall_height 64
static const unsigned char WristKeyLogoSmall_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0xf8, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff,
   0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0x0f, 0x00, 0x00,
   0x00, 0x00, 0xfe, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
   0xff, 0xff, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xf0, 0xff, 0xff,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x00,
   0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
   0x80, 0xff, 0x7f, 0x00, 0x00, 0xfe, 0xff, 0x01, 0xc0, 0xff, 0x07, 0x00,
   0x00, 0xe0, 0xff, 0x03, 0xe0, 0xff, 0x03, 0x00, 0x00, 0xc0, 0xff, 0x07,
   0xe0, 0xff, 0x01, 0x00, 0x00, 0x80, 0xff, 0x07, 0xf0, 0xff, 0x00, 0x00,
   0x00, 0x00, 0xff, 0x0f, 0xf8, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x1f,
   0xf8, 0xff, 0x00, 0x00, 0xfc, 0x01, 0xff, 0x1f, 0xf8, 0xff, 0x00, 0x00,
   0xfe, 0x03, 0xff, 0x1f, 0xfc, 0xff, 0x00, 0x80, 0x3f, 0x06, 0xff, 0x3f,
   0xfc, 0xff, 0x01, 0x80, 0x3f, 0x8e, 0xff, 0x3f, 0xfe, 0xff, 0x01, 0x80,
   0x3f, 0x8e, 0xff, 0x7f, 0xfe, 0xff, 0x01, 0xc0, 0xff, 0x8f, 0xff, 0x7f,
   0xfe, 0xff, 0x01, 0xc0, 0xff, 0x9f, 0xff, 0x7f, 0xf6, 0xdf, 0x01, 0xc0,
   0xff, 0x9f, 0xe7, 0x73, 0xf6, 0xdf, 0x01, 0xc0, 0xff, 0x8f, 0xe7, 0x79,
   0xe7, 0xcf, 0x01, 0x80, 0xff, 0x8f, 0xe7, 0xf9, 0xe7, 0xce, 0x01, 0x80,
   0xff, 0x8f, 0xe7, 0xfc, 0x67, 0xcc, 0x01, 0xc0, 0xff, 0x87, 0x67, 0xfe,
   0x67, 0xcc, 0x01, 0xc0, 0xff, 0x83, 0x27, 0xff, 0x27, 0xc9, 0x01, 0xc0,
   0xff, 0x80, 0x27, 0xff, 0x27, 0xc9, 0x00, 0xe0, 0x0f, 0x00, 0x87, 0xff,
   0x2f, 0xe9, 0x00, 0xf0, 0x03, 0x00, 0x27, 0xff, 0x8f, 0xe3, 0x00, 0xf8,
   0x01, 0x00, 0x27, 0xfe, 0x8f, 0xe3, 0x00, 0x78, 0x00, 0x00, 0x67, 0xfc,
   0x8f, 0x63, 0x00, 0x7c, 0x00, 0x00, 0xe6, 0xf8, 0x8e, 0x63, 0x00, 0x3e,
   0x00, 0x00, 0xe6, 0x79, 0xce, 0x67, 0x00, 0x1e, 0x00, 0x00, 0xe6, 0x73,
   0xce, 0x67, 0x00, 0x1f, 0x00, 0x00, 0xe6, 0x63, 0xfe, 0x7f, 0x80, 0x0f,
   0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0xc0, 0x07, 0x00, 0x00, 0xfe, 0x7f,
   0xfc, 0x7f, 0xc0, 0x07, 0x00, 0x00, 0xfe, 0x3f, 0xfc, 0xff, 0xe0, 0x03,
   0x00, 0x00, 0xff, 0x3f, 0xf8, 0xff, 0xe0, 0x01, 0x00, 0x00, 0xff, 0x1f,
   0xf8, 0xff, 0xc0, 0x01, 0x00, 0x00, 0xff, 0x1f, 0xf8, 0xff, 0x01, 0x00,
   0x00, 0x80, 0xff, 0x1f, 0xf0, 0xff, 0x03, 0x00, 0x00, 0xc0, 0xff, 0x0f,
   0xe0, 0xff, 0x07, 0x00, 0x00, 0xe0, 0xff, 0x07, 0xe0, 0xff, 0x0f, 0x00,
   0x00, 0xf0, 0xff, 0x07, 0xc0, 0xff, 0x3f, 0x00, 0x00, 0xfc, 0xff, 0x03,
   0x80, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x01, 0x00, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x3f, 0xfc, 0xff, 0xff, 0x00,
   0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0xfc, 0xff, 0xff,
   0xff, 0xff, 0x3f, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xc0, 0xff, 0xff,
   0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
   0x00, 0x00, 0xfe, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff,
   0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x03, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xf8, 0x1f, 0x00, 0x00, 0x00
};

void logo() {
  u8g2.clearBuffer();
  u8g2.drawXBMP(32, 0, WristKeyLogoSmall_width, WristKeyLogoSmall_height, WristKeyLogoSmall_bits);
  u8g2.sendBuffer();
  delay(5000);
}
