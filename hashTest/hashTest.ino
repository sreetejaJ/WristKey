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

    char *payload = "SecurePassword42";                                         //This creates the password and then commits the hash to EEPROM at addr 0.
    byte shaResult[32];
  
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
  
    const size_t payloadLength = strlen(payload);
  
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (unsigned char *) payload, payloadLength);
    mbedtls_md_finish(&ctx, shaResult);
    mbedtls_md_free(&ctx);
  
    Serial.print("Hash: ");
  
    for (int i = 0; i < sizeof(shaResult); i++)
    {
      char str[3];
      sprintf(str, "%02x", (int)shaResult[i]);
      Serial.print(str);
      EEPROM.write(i, shaResult[i]);
    }
    EEPROM.commit();
    payload = "APPLES";                                         //This creates the password and then commits the hash to EEPROM at addr 0.
  
    payloadLength = strlen(payload);
  
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (unsigned char *) payload, payloadLength);
    mbedtls_md_finish(&ctx, shaResult);
    mbedtls_md_free(&ctx);
  
    Serial.print("Hash: ");
  
    for (int i = 0; i < sizeof(shaResult); i++)
    {
      char str[3];
      sprintf(str, "%02x", (int)shaResult[i]);
      Serial.print(str);
      EEPROM.write(i + 32, shaResult[i]);
    }
    Serial.println("\nHash commited to memory");
    
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

