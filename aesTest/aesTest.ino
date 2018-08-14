#include "mbedtls/aes.h"
#include "EEPROM.h"


#define EEPROM_SIZE 4096

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.print("failed to initialise EEPROM"); delay(1000000);
  }
  
  int repeat = storeData("abcdefghijklmnop", "Encrypt This boy. Padding:");

  for(int i = 0; i < repeat; i++){
    
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:

}


int storeData(char * key, char * input) {

  if (strlen(key) != 16) {
    return 0;
  }
  unsigned int repeats = strlen(input) / 16;
  unsigned int extra = strlen(input) % 16;
  for (int i = 0; i <= repeats; i++) {
    unsigned char output[16];
    char in[16];
    for(int j = 0; j < 16; j++){
      if(j >= extra && i == repeats){
        in[j] = 35;
      }else{
        in[j] = input[j + i* 16];
      }
    }
    mbedtls_aes_context aes;
    mbedtls_aes_init( &aes );
    mbedtls_aes_setkey_enc( &aes, (const unsigned char*) key, strlen(key) * 8 );
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)in, output);
    mbedtls_aes_free( &aes );
    for(int i = 0; i < 16; i++){
      Serial.print(output[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  return repeats + 1;
}

