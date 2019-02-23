/*
   EEPROM Write

   Stores random values into the EEPROM.
   These values will stay in the EEPROM when the board is
   turned off and may be retrieved later by another sketch.
*/

#include "EEPROM.h"

// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
#define EEPROM_SIZE 4096
void setup()
{
  Serial.begin(115200);
  Serial.println("start...");
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
  Serial.println(" bytes read from Flash . Values are:");
  for(int i = 0; i < EEPROM_SIZE; i++){
      Serial.print(EEPROM.read(i), HEX);
      Serial.print(" ");
      if((i % 16) == 0){
        Serial.println();
      }
    }
  Serial.println();

}

void loop()
{
}
