void storeData(char * key, char * webHash, char * username, char * password, unsigned int addr) {
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
  addr++;
#ifdef DEBUG
  Serial.print("Username: ");
#endif
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
    Serial.println(addr - LenAddr - 1);
#endif
  }

  EEPROM.write(LenAddr, addr - LenAddr - 1);
#ifdef DEBUG
  Serial.println(addr);
#endif
  EEPROM.write(dataLenAddr, addr - dataLenAddr - 1);
  EEPROM.commit();
}

void pullData(unsigned int addr, String &password, String &username, char * key) {
  
  int dataSize = EEPROM.read(addr + 1);
  addr += 2;
  Serial.println("Size of data is: ");
  Serial.print(dataSize);
  Serial.println();
  Serial.println("The website hash is: ");
  for (int i = 0; i < 32; i++) {
    Serial.print(EEPROM.read(addr + i), HEX);
    Serial.print(" ");
  }
  addr += 32;
  Serial.println();
  int Len = EEPROM.read(addr);
  Serial.printf("The encrypted password length is: %d\n", Len);
  addr += 1;
  int repeats = Len / 16;
  for (int j = 0; j < repeats; j++) {
    unsigned char cipherText[16];
    unsigned char decipheredText[16];
    for (int i = 0; i < 16; i++) {
      Serial.print(EEPROM.read(addr + i + j * 16), HEX);
      cipherText[i] = char(EEPROM.read(addr + i + j * 16));
      Serial.print(" ");
    }
    decrypt(cipherText, key, decipheredText);
    for (int i = 0; i < 16; i++) {
      if(decipheredText[i] != 35){
        password += (char) decipheredText[i];
      }
    }
  }
  addr += Len;
  Len = EEPROM.read(addr);
  Serial.printf("The encrypted username length is: %d\n", Len);
  addr += 1;
  repeats = Len / 16;
  for (int j = 0; j < repeats; j++) {
    unsigned char cipherText[16];
    unsigned char decipheredText[16];
    for (int i = 0; i < 16; i++) {
      Serial.print(EEPROM.read(addr + i + j * 16), HEX);
      cipherText[i] = char(EEPROM.read(addr + i + j * 16));
      Serial.print(" ");
    }
    decrypt(cipherText, key, decipheredText);
    for (int i = 0; i < 16; i++) {
      if(decipheredText[i] != 35){
        username += (char) decipheredText[i];
      }
    }
  }
}


String getCode() {
  int pos = 0;
  int lastPos = 0;
  String tapCode = "##";
  byte tapPos = 0;
  while (true) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_17_tf);
    u8g2.setCursor(7, 15);
    u8g2.print("Enter Code");
    for (int i = 0; i < 6; i++) {
      u8g2.setCursor(40 + i * 15, 42);
      if (tapPos > i) {
        u8g2.print("*");
      }
      u8g2.drawFrame(38 + i * 15, 31, 12, 13);
    }
    u8g2.setFont(u8g2_font_logisoso28_tf);
    u8g2.setCursor(5, 55);
    u8g2.print(characters[pos]);
    u8g2.sendBuffer();
    while (pos == lastPos) {
      int val = getInput();
      if (val == 10) {
        tapCode += characters[pos];
        tapCode += "#";
        if (tapPos >= 5) {
          u8g2.setCursor(40 + 5 * 15, 42);
          u8g2.print("*");
          delay(80);
          tapCode += "##";
          return tapCode;
        }
        tapPos++;
        break;
      }
      pos += val;
    }
    if (pos > 25) {
      pos = 0;
    }
    if (pos < 0) {
      pos = 25;
    }
    lastPos = pos;
  }
}

bool Code() {
  byte tries = 0;
  while (tries < 3) {
    key = getCode();
    if (checkHash(key, 0x20)) {
#ifdef DEBUG
      Serial.println("Code Successful");
#endif
      return true;
    } else {
#ifdef DEBUG
      Serial.println("Code Unsuccessful");
#endif
      tries++;
    }
  }
  return false;
}

int NextAvailAddr() {
  for (int i = PACKETS_START_ADDR; i < EEPROM_SIZE; i++) {
    if (EEPROM.read(i) == 0x00) {
      i += EEPROM.read(i + 1);
    }
    if (EEPROM.read(i) == 0xFF) {
      return i;
    }
  }
}

int findHash(String value) {
  int addr = PACKETS_START_ADDR;
  while (addr < EEPROM_SIZE) {
#ifdef DEBUG
    //Serial.printf("Checking addr: %d\n", addr);
#endif
    if (EEPROM.read(addr) == 0x00) {
      int len = EEPROM.read(addr + 1);
      if (checkHash(value, addr + 2)) {
        return addr;
      } else {
        addr += len + 1;
      }
    } else {
      addr++;
    }
  }
  return -1;
}

void decrypt(unsigned char * chipherText, char * key, unsigned char * outputBuffer) {

  mbedtls_aes_context aes;

  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_dec( &aes, (const unsigned char*) key, strlen(key) * 8 );
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char*)chipherText, outputBuffer);
  mbedtls_aes_free( &aes );
}

char* convertString(String in){
    if(in.length()!=0){
        char *val = const_cast<char*>(in.c_str());
        return val;
    }
}
