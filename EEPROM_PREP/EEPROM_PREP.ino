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
