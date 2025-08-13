#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
byte nuidPICC[4];
 
void setup(void) 
{
 Serial.begin(115200);
 Serial.println("System initialized");
 nfc.begin();
 Wire.begin(21, 22);
 Wire.setClock(100000);
}
 
void loop() 
{
 readNFC();

  if(tagId == "B3 09 5F 1A") {
   Serial.println("TARJETA");
 } else if(tagId == ""){
   Serial.println("NO TARJETA");
 } else {
    Serial.println("TARJETA QUE NO NOS IMPORTA");
 }
}
 
void readNFC() 
{
 if (nfc.tagPresent())
 {
   NfcTag tag = nfc.read();
   tag.print();
   tagId = tag.getUidString();
 }
 delay(1000);
}
