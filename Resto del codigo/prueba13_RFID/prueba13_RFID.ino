#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5    // SDA en el RC522
#define RST_PIN 22  // RST en el RC522

MFRC522 mfrc522(SS_PIN, RST_PIN); // Crear el objeto del lector

void setup() {
  Serial.begin(115200);
  SPI.begin();        // Iniciar bus SPI
  mfrc522.PCD_Init(); // Iniciar RC522
  Serial.println("Lector RFID listo. Acerca una tarjeta...");
}

void loop() {
  // Ver si hay nueva tarjeta
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Ver si se puede leer
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Mostrar UID
  Serial.print("UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  String uid = getUID();
  Serial.print("UID leído: ");
  Serial.println(uid);

  if(uid == "A92A7341") {
    Serial.println("tarjeta no autorizada");
  } else if (uid == "593D8747") {
    Serial.println("tarjeta autorizada");
  }
}


String getUID() {
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += String(mfrc522.uid.uidByte[i], HEX); // lo convierte a HEX
  }
  uidString.toUpperCase(); // opcional, para que siempre sea en mayúsculas
  return uidString;
}