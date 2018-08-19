#include "EEPROM.h"
#include "mbedtls/md.h"

#define EEPROM_SIZE 4096

int addr = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.print("failed to initialise EEPROM"); delay(1000000);
  }

    char *payload = "SecurePassword42";                                         //This creates the password and then commits the hash to EEPROM at addr 0.
    byte shaResult[32];
  
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
  
    size_t payloadLength = strlen(payload);
  
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
      //EEPROM.write(i + addr, shaResult[i]);
    }
    addr += sizeof(shaResult);
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
      EEPROM.write(i + addr, shaResult[i]);
    }
    addr += sizeof(shaResult);

    for(;addr < EEPROM_SIZE; addr++){
      EEPROM.write(addr, 0xFF);
    }
    EEPROM.commit();
    Serial.println("\nHash commited to memory\n\n");
    for(int i = 0; i < EEPROM_SIZE; i++){
      Serial.print(EEPROM.read(i), HEX);
      Serial.print(" ");
      if((i % 16) == 0){
        Serial.println();
      }
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}
