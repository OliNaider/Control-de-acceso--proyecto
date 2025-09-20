#include <Keypad.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
/*#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);*/


//CERRADURA
#define PIN_CERRADURA 4

//RFID
#define SS_PIN 5    // SDA en el RC522
#define RST_PIN 22  // RST en el RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); // Crear el objeto del lector

//KEYPAD
#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // three columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM] = {32, 33, 25, 26}; // GPIO18, GPIO5, GPIO17, GPIO16 connect to the row pins
byte pin_column[COLUMN_NUM] = {27, 14, 12, 13};  // GPIO4, GPIO0, GPIO2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
 
String input_password;
const String cambioClave = "123";
String password = "7890";
int estadoK = 0;
int intentos = 0;

unsigned long tiempoDeInicio = 0; 
unsigned long duracionBloqueo = 5000;
unsigned long duracionCambios = 10000;

//tiempo cerradura 
int estadoC = 0;
unsigned long tiempoCerradura = 0;
unsigned long duracionCerradura = 2000;

//NFC
const int MAX_TAGS = 20; // máximo de tarjetas que se pueden guardar
String authorizedTags[MAX_TAGS]; // array de UIDs
int authorizedIDs[MAX_TAGS];            // array paralelo de IDs
int tagCount = 0;                // cuántas tarjetas hay guardadas
int nextID = 1;                  // próximo ID a asignar
String uid = "";
int estadoN = 0;
int ID = 0; 

void setup() {
  Serial.begin(115200);
  input_password.reserve(32); // maximum input characters is 33 (keypad)
  Wire.begin();

  pinMode(PIN_CERRADURA, OUTPUT);

  //RFID
  SPI.begin();        // Iniciar bus SPI
  mfrc522.PCD_Init(); // Iniciar RC522
  Serial.println("Lector RFID listo. Acerca una tarjeta...");

  //LCD
  /*lcd.init();
  lcd.clear();
  lcd.backlight();  */
}

void loop() {
  char key = keypad.getKey();
 
  if(estadoK == 3) {
    if((millis() - tiempoDeInicio) >= duracionBloqueo) {
      estadoK = 0;
      intentos = 0;
      Serial.println("sistema desbloqueado");

    } else {
      Serial.println("SISTEMA BLOQUEADO");
    }
    return;
  }

  if (estadoK == 1 && (millis() - tiempoDeInicio > duracionCambios)) {
    Serial.println("Tiempo agotado, volviendo a estado 0");
    estadoK = 0;
  }

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    uid = getUID();
    Serial.print("UID leído: ");
    Serial.println(uid);
    delay(1000);
    estadoN = 1; 
  }

  if (key) {
    
    Serial.println(key);  
    //lcd.print("*");

    switch (key) {

      case '*':
        input_password = ""; // limpiar input password
       // lcd.clear();
        Serial.println(password);
        estadoK = 0; 
        break;

      case '#':
        if(estadoK == 0) {
          if (password == input_password) {
            Serial.println("The password is correct");
            /*lcd.clear();
            lcd.print("The password is");
            lcd.setCursor(2, 1);
            lcd.print("correct");
            delay(500);
            lcd.clear();*/

            digitalWrite(PIN_CERRADURA, HIGH); //apagar la cerradura y que se abra la puerta
            intentos = 0; 

            Serial.println(password);
            
            tiempoDeInicio = millis();
            tiempoCerradura = millis();

            estadoC = 1; 
            estadoK = 1;
            
            Serial.println(estadoK);
            Serial.print("estadoC:");
            Serial.println(estadoC);

          } else {
            Serial.println("The password is incorrect");
            /*lcd.clear();
            lcd.print("The password is");
            lcd.setCursor(2, 1);
            lcd.print("incorrect");
            delay(500);
            lcd.clear();*/


            intentos++;
            Serial.println(intentos);
            if(intentos >= 5){
              estadoK = 3;
              tiempoDeInicio = millis();
            } else {
              estadoK = 0;
              Serial.println(estadoK);
              Serial.println(password);
            }

            estadoK = 0; 
            estadoC = 0; 

          }

          input_password = "";

        } else if(estadoK == 1){
          if(input_password == "123"){
            //ingresar nueva clave
            Serial.println("ingrese la nueva clave");
            estadoK = 2;
            Serial.println(password);
            Serial.println(estadoK);


          } else if (input_password == "456") {
            //acreditar tarjeta y asignarle un ID
            while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
            }  
            uid = getUID();
            Serial.print("UID leído: ");
            Serial.println(uid);
            int BLABLA = encontrarTag(uid);
            if(BLABLA == -1) {
              Serial.println("la tarjeta es nueva");

              if (tagCount < MAX_TAGS) {
                Serial.println("autorizando tarjeta");
                authorizedTags[tagCount] = uid; 
                Serial.println();
                Serial.print("tarjeta autorizada: ");
                Serial.println(authorizedTags[tagCount]);
                authorizedIDs[tagCount] = nextID;
                Serial.print("con el ID: ");
                Serial.println(authorizedIDs[tagCount]);

                nextID++;
                tagCount++;
                estadoK = 0;
                uid = "";
              }  

            } else {
              Serial.println("Esta tarjeta ya esta autorizada");
              estadoK = 0;
            }
            
            estadoK = 0;
            delay(1000);

          } else if(input_password == "ABC"){
            //desacreditar tarjeta 
            Serial.println("Ingrese ID de la tarjeta que desea eliminar");
            estadoK = 4;
            input_password = "";


          } else if(input_password == "BCD"){
            //mostrar ID
            Serial.println("acerque la tarjeta");
            while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
            }  

            uid = getUID();
            Serial.println(uid);
            int BLABLA = encontrarTag(uid);
            if(BLABLA == -1) {
              Serial.println("Tarjeta sin ID asignado");
              delay(5000);
              input_password = "";
              estadoK = 0;           
            } else {
              encontrarID(uid);
              Serial.print("Con el ID: ");
              Serial.println(ID);
              delay(5000);
              input_password = "";
              estadoK = 0; 
            }


          }else {
            estadoK = 0;
            Serial.println("no se ha cambiado la clave");
            Serial.println(password);
            Serial.println(estadoK);
          }

          input_password = "";

        }else if(estadoK == 2) {
          password = input_password;
          Serial.println(password);
          Serial.println("clave cambiada");
          Serial.println(password);
          estadoK = 0;
          Serial.println(estadoK);

          input_password = "";
        } else if(estadoK == 4) {
          int eliminarID = input_password.toInt();
          eliminarTag(eliminarID);
          estadoK = 0;
          input_password = "";
        }

        break;

      default:
        input_password += key; // agregar carácter a input password
        break;
    } 
  }

  if(estadoC == 1 && (millis() - tiempoCerradura > duracionCerradura)){
    Serial.println("cerradura cerrada");
    digitalWrite(PIN_CERRADURA, LOW);
    estadoC = 0; 
  }
  
  if(estadoN == 1){
    int index = encontrarTag(uid);
    if(index != -1) {
      Serial.print("TARJETA CORRECTA. INGRESE!!!!");
      Serial.println(authorizedTags[index]);

      digitalWrite(PIN_CERRADURA, HIGH);

      uid = "";
      estadoN = 0;
    } else {
      Serial.println("TARJETA INCORRECTA");
      uid = "";
      estadoN = 0;
    }
  } 


}



//FUNCIONES RFID
String getUID() {
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += String(mfrc522.uid.uidByte[i], HEX); // lo convierte a HEX
  }
  uidString.toUpperCase(); // opcional, para que siempre sea en mayúsculas
  return uidString;
}

int encontrarTag(String tag) {
  for(int i = 0; i < tagCount; i++) {
    if(authorizedTags[i] == tag) {
      return i;  // encontrada
    }
  }
  return -1; // no encontrada
}

void leerTag() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    uid = getUID();
    Serial.print("UID leído: ");
    Serial.println(uid);
    delay(1000);
  }
  delay(1000);
}

void eliminarTag(int ID) {
  for(int i = 0; i < tagCount; i++) {
    if(authorizedIDs[i] == ID) {
      for (int j = i; j < tagCount - 1; j++) {
        authorizedTags[j] = authorizedTags[j + 1];
        authorizedIDs[j] = authorizedIDs[j + 1];
      }
      tagCount--;
      Serial.println("Eliminacion completada");
    }
  }
}

void encontrarID(String tag) {
  for(int i = 0; i < tagCount; i++) {
    if (authorizedTags[i] == tag) {
      ID = authorizedIDs[i];
    }
  }
}


