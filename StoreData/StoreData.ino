#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include "EEPROM.h"


#define EEPROM_SIZE 4096
//#define DEBUG 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.print("failed to initialise EEPROM"); delay(1000000);
  }
  pinMode(LED_BUILTIN, OUTPUT);

  int addr = 0x00;
  int repeat = storeData("abcdefghijklmnop", "www.google.com", "SecurePassword42", "letmein", addr);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:

}

int storeData(char * key, char * webHash, char * password, char * username, unsigned int addr) {
  //**************************************************************DATA HEADER**********************************************************
  EEPROM.write(addr, 0x00);                                                                       //Start with NULL
  int dataLenAddr = addr + 1;                                                                     //Save byte for data len
  addr += 2;
#ifdef DEBUG
  Serial.println(addr);
#endif
  //**************************************************************Website Hash*********************************************************
  byte shaResult[32];

  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  const size_t payloadLength = strlen(webHash);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (unsigned char *) webHash, payloadLength);
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);
#ifdef DEBUG
  Serial.print("Hash: ");
#endif
  for (int i = 0; i < sizeof(shaResult); i++)
  {
#ifdef DEBUG
    char str[3];
    sprintf(str, "%02x", (int)shaResult[i]);
    Serial.print(str);
#endif
    EEPROM.write(addr, shaResult[i]);
    addr++;
  }
#ifdef DEBUG
  Serial.println();
  Serial.println(addr);
#endif
  //**************************************************************Password Encrypter**********************************************************
  unsigned int repeats = strlen(password) / 16;
  unsigned int extra = strlen(password) % 16;
  unsigned int LenAddr = addr;
  addr++;
#ifdef DEBUG
  Serial.print("Password: ");
#endif
  for (int i = 0; i <= repeats; i++) {
    unsigned char output[16];
    char in[16];
    for (int j = 0; j < 16; j++) {
      if (j >= extra && i == repeats) {
        in[j] = 35;
      } else {
        in[j] = password[j + i * 16];
      }
    }
    mbedtls_aes_context aes;
    mbedtls_aes_init( &aes );
    mbedtls_aes_setkey_enc( &aes, (const unsigned char*) key, strlen(key) * 8 );
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)in, output);
    mbedtls_aes_free( &aes );
    for (int k = 0; k < 16; k++) {
      EEPROM.write(addr, output[k]);
      addr++;
#ifdef DEBUG
      Serial.print(output[k], HEX);
      Serial.print(" ");
#endif
    }
#ifdef DEBUG
    Serial.println();
#endif
  }
#ifdef DEBUG
  Serial.println(addr);
#endif
  EEPROM.write(LenAddr, addr - LenAddr - 1);
  //**************************************************************Username Encrypter**********************************************************
  repeats = strlen(username) / 16;
  extra = strlen(username) % 16;
  LenAddr = addr;
#ifdef DEBUG
  Serial.print("Username: ");
#endif
  addr++;
  for (int i = 0; i <= repeats; i++) {
    unsigned char output[16];
    char in[16];
    for (int j = 0; j < 16; j++) {
      if (j >= extra && i == repeats) {
        in[j] = 35;
      } else {
        in[j] = username[j + i * 16];
      }
    }
    mbedtls_aes_context aes;
    mbedtls_aes_init( &aes );
    mbedtls_aes_setkey_enc( &aes, (const unsigned char*) key, strlen(key) * 8 );
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)in, output);
    mbedtls_aes_free( &aes );
    for (int k = 0; k < 16; k++) {
      EEPROM.write(addr, output[k]);
      addr++;
#ifdef DEBUG
      Serial.print(output[k], HEX);
      Serial.print(" ");
#endif
    }
#ifdef DEBUG
    Serial.println();
#endif
  }
#ifdef DEBUG
  Serial.println(addr);
#endif
  EEPROM.write(dataLenAddr, addr - dataLenAddr - 1);
  EEPROM.commit();
}
