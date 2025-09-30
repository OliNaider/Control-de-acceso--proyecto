#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

String tagId = "None";
String arrayTags[10];   // array para guardar hasta 10 UIDs
int indexTag = 0;       // posición actual en el array

void setup(void) 
{
  Serial.begin(115200);
  Serial.println("System initialized");

  Wire.begin(21, 22);    
  Wire.setClock(100000);

  nfc.begin();
}
 
void loop() 
{
  readNFC();

  if (tagId == "B3 09 5F 1A") {
    Serial.println("TARJETA AUTORIZADA ✅");
  } else if (tagId == "") {
    Serial.println("NO HAY TARJETA");
  } else {
    Serial.println("TARJETA NO AUTORIZADA ❌");
  }

  // Mostrar todos los UIDs guardados hasta ahora
  Serial.println("Tags almacenados:");
  for (int i = 0; i < indexTag; i++) {
    Serial.println(arrayTags[i]);
  }

  delay(500);
}
 
void readNFC() 
{
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    tag.print();
    tagId = tag.getUidString();

    // Guardar en el array si hay espacio
    if (indexTag < 10) {
      arrayTags[indexTag] = tagId;
      indexTag++;
    }
  } else {
    tagId = "";  // cuando no hay tarjeta
  }
}

