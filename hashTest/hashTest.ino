#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "EEPROM.h"
#include "mbedtls/md.h"

#define EEPROM_SIZE 4096
#define TOUCH_SENSITIVITY 40

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  u8g2.begin();

  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.print("failed to initialise EEPROM"); delay(1000000);
  }

  //  char *payload = "012012";                                         //This creates the password and then commits the hash to EEPROM at addr 0.
  //  byte shaResult[32];
  //
  //  mbedtls_md_context_t ctx;
  //  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
  //
  //  const size_t payloadLength = strlen(payload);
  //
  //  mbedtls_md_init(&ctx);
  //  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  //  mbedtls_md_starts(&ctx);
  //  mbedtls_md_update(&ctx, (unsigned char *) payload, payloadLength);
  //  mbedtls_md_finish(&ctx, shaResult);
  //  mbedtls_md_free(&ctx);
  //
  //  Serial.print("Hash: ");
  //
  //  for (int i = 0; i < sizeof(shaResult); i++)
  //  {
  //    char str[3];
  //    sprintf(str, "%02x", (int)shaResult[i]);
  //    Serial.print(str);
  //    EEPROM.write(i, shaResult[i]);
  //  }
  //  EEPROM.commit();
  //  Serial.println("\nHash commited to memory");
  //Serial.println("\n\nEnter passphrase\n\n");

  bool unlocked = false;
  int index = 0;
  char keyAttempt[5];
  while (!unlocked) {
    int Lpad = touchRead(T4);
    int Cpad = touchRead(T7);
    int Rpad = touchRead(T9);
    Serial.print(Lpad);
    Serial.print("\t");
    Serial.print(Cpad);
    Serial.print("\t");
    Serial.print(Rpad);
    Serial.print("\t");
    Serial.print(index);
    Serial.print("\n");
    
    if (index == 5) {
      Serial.println(keyAttempt);
      if (!checkHash(keyAttempt)) {
        index = 0;
      } else {
        unlocked = true;
        break;
      }
    }

    if (Lpad < TOUCH_SENSITIVITY) {
      keyAttempt[index] = 0;
      index++;
      while(Lpad < TOUCH_SENSITIVITY){
        Lpad = touchRead(T4);
      }
      
    } else if (Cpad < TOUCH_SENSITIVITY) {
      keyAttempt[index] = 1;
      index++;
      while(Cpad < TOUCH_SENSITIVITY){
        Cpad = touchRead(T7);
      }
      
    } else if (Rpad < TOUCH_SENSITIVITY) {
      keyAttempt[index] = 2;
      index++;
      while(Rpad < TOUCH_SENSITIVITY){
        Rpad = touchRead(T9);
      }
  
    }
    Serial.print("\t\t End of checks");
    delay(100);
  }
}

void loop() {
  Serial.println("WELCOME TO WRISTKEY!!");
  delay(100);
}



bool checkHash(char attempt[]) {
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


  bool hashCorrect = true;
  for (int i = 0; i < sizeof(shaResult); i++) {
    Serial.println(shaResult[i]);
    if (EEPROM.read(i) != shaResult[i]) {
      return false;
    }
  }
  return true;
}

