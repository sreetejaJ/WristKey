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

String getCode() {
  int pos = 0;
  int lastPos = 0;
  String tapCode = "";
  byte tapPos = 0;
  while (true) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_17_tf);
    u8g2.setCursor(7, 15);
    u8g2.print("Enter Code");
    for (int i = 0; i < 6; i++) {
      u8g2.setCursor(40 + i * 15, 42);
      if (tapCode.length() > i) {
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
        if (tapPos >= 5) {
          u8g2.setCursor(40 + 5 * 15, 42);
          u8g2.print("*");
          delay(80);
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

bool checkHash(char attempt[], int startAddr) {
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


  Serial.println("Hash Created");
  for (int i = 0; i < sizeof(shaResult); i++) {
    Serial.println(shaResult[i], HEX);
    if (EEPROM.read(i + startAddr) != shaResult[i]) {
      return false;
    }
  }
  return true;
}

bool Code() {
  byte tries = 0;
  while (tries < 3) {
    char attempt[7];
    String val = getCode();
    val.toCharArray(attempt, 7);
    Serial.println(val);
    for(int i = 0; i < 7; i++){
      Serial.println(attempt[i], HEX);
    }
    if (checkHash(attempt, 0x20)) {
      Serial.println("Code Successful");
      return true;
    }else{
      Serial.println("Code Unsuccessful");
      tries++;
    }
  }
  return false;
}
