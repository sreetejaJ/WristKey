void pullData(unsigned int addr, String &password, String &username) {
  int dataSize;
  for (int i = 0; i < EEPROM_SIZE; i++) {
    if (EEPROM.read(i) == NULL) {
      dataSize = EEPROM.read(i + 1);
      addr = i + 2;
      break;
    }
  }
#ifdef SERIAL
  Serial.println("Size of data is: ");
  Serial.print(dataSize);
  Serial.println();
  Serial.println("The website hash is: ");
  for (int i = 0; i < 32; i++) {
    Serial.print(EEPROM.read(addr + i), HEX);
    Serial.print(" ");
  }
#endif
  addr += 32;
  int Len = EEPROM.read(addr);
#ifdef SERIAL
  Serial.println();
  Serial.println("The encrypted password length is: ");
  Serial.print(Len);
  Serial.println();
#endif
  addr += 1;
  for (int i = 0; i < Len; i++) {
#ifdef SERIAL
    Serial.print(EEPROM.read(addr + i), HEX);
    Serial.print(" ");
#endif
    password += String(EEPROM.read(addr + i));
  }
  addr += 32;
#ifdef SERIAL
  Serial.println();
  Serial.println("The encrypted username length is: ");
  Serial.print(Len);
  Serial.println();
#endif
  Len = EEPROM.read(addr);
  addr += 1;
  for (int i = 0; i < Len; i++) {
#ifdef SERIAL
    Serial.print(EEPROM.read(addr + i), HEX);
    Serial.print(" ");
#endif
    username += String(EEPROM.read(addr + i));
  }
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
#ifdef SERIAL
    Serial.println(shaResult[i]);
#endif
    if (EEPROM.read(i) != shaResult[i]) {
      return false;
    }
  }
  return true;
}

char * returnEncrypted(char * input, char * key, unsigned int repeats, unsigned int extra, unsigned int addr) {
  Serial.println("returnEncrypted called!");
  if (input == NULL || key == NULL) {
    return NULL;
  }
#ifdef SERIAL
  Serial.println("Input validation complete!");
#endif
  unsigned int sizeOf = repeats;
  if (extra > 0) {
    sizeOf = repeats + 1;
  }

  if (sizeOf <= 0) {
    return NULL;
  }
  char cipherOut[sizeOf * 16];

  for (int i = 0; i <= repeats; i++) {
    unsigned char output[16];
    char in[16];
    for (int j = 0; j < 16; j++) {
      if (j >= extra && i == repeats) {
        in[j] = 35;
      } else {
        in[j] = input[j + i * 16];
      }
    }
    mbedtls_aes_context aes;
    mbedtls_aes_init( &aes );
    mbedtls_aes_setkey_enc( &aes, (const unsigned char*) key, strlen(key) * 8 );
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)in, output);
    mbedtls_aes_free( &aes );
    for (int k = 0; k < 16; k++) {
      cipherOut[k + i * 16] = output[k];
#ifdef SERIAL
      Serial.print(output[k], HEX);
      Serial.print(" ");
#endif
    }
#ifdef SERIAL
    Serial.println();
#endif
  }
  return cipherOut;
}

int getNextAvailAddr(){
  int addr = PACKETS_START_ADDR;
  while(addr < EEPROM_SIZE){
    if(EEPROM.read(addr) == NULL){
      addr += EEPROM.read(addr + 1) + 1;
    }
  }
}

