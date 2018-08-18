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

  int addr = 0x21;
  int repeat = storeData("abcdefghijklmnop", "Encrypt This boy. Padding:", "asdf", "asdf", addr);
}

void loop() {
  // put your main code here, to run repeatedly:

}


int storeData(char * key, char * webHash, char * password, char * username, unsigned int addr) {
  Serial.println("storeData called");
  if (strlen(key) != 16) {
    return 0;
  }
  EEPROM.write(addr, 0x00);                                                                       //Start with NULL
  int dataLenAddr = addr + 1;                                                                         //Save byte for data len
  addr += 2;

  // Website Hash









  Serial.println("key was checked and is fine!");
  unsigned int Prepeats = strlen(password) / 16;
  unsigned int Pextra = strlen(password) % 16;

  unsigned int pwordLen = Prepeats;
  if(Pextra > 0){
    pwordLen += 1;
  }

  unsigned int Urepeats = strlen(username) / 16;
  unsigned int Uextra = strlen(username) % 16;

  unsigned int uNameLen = Urepeats;
  if(Uextra > 0){
    uNameLen += 1;
  }
  Serial.println(pwordLen);
  char * encrypted = returnEncrypted(password, key, Prepeats, Pextra);
  Serial.println("Finished the encryption");
  Serial.print(encrypted);


  return 1;
}


char * returnEncrypted(char * input, char * key, unsigned int repeats, unsigned int extra){
  Serial.println("returnEncrypted called!");
  if(input == NULL || key == NULL){
    return NULL;
  }

  Serial.println("Input validation complete!");

  unsigned int sizeOf = repeats;
  if(extra > 0){
    sizeOf = repeats + 1;
  }

  if(sizeOf <= 0){
    return NULL;
  }
  char cipherOut[sizeOf * 16];

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
    for(int k = 0; k < 16; k++){
      Serial.print(output[k], HEX);
      cipherOut[k + i * 16] = output[k];
      Serial.print(" ");
    }
    Serial.println();
  }
  return cipherOut;
}
