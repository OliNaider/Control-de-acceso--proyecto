#include <Keypad.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <esp_now.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//WIFI
const char* ssid = "CARRO 2601 2.4GHz";
const char* passwordWIFI = "colchones301";

//ESP-NOW
uint8_t broadcastAddress[] = {0xB0, 0xA7, 0x32, 0xF1, 0xD7, 0xA4};  // Dirección MAC del receptor (ESP32-CAM)

typedef struct estructura {  // Estructura del mensaje
  char msg[32];
} estructura;
estructura myData;

String datos = ""; 
void mensaje1();

//CERRADURA
#define PIN_CERRADURA 13

//RFID
#define SS_PIN 5    // SDA en el RC522
#define RST_PIN 22  // RST en el RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); // Crear el objeto del lector

//KEYPAD
#define ROW_NUM     4 
#define COLUMN_NUM  4 

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM] = {32, 33, 25, 26}; // GPIO18, GPIO5, GPIO17, GPIO16 connect to the row pins
byte pin_column[COLUMN_NUM] = {27, 14, 16, 4};  // GPIO4, GPIO0, GPIO2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
 
String input_password;
const String cambioClave = "123";
String password = "1515";
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

//FINAL DE CARRERAS
const int PIN_finalCarreras = 12;
unsigned long ultimoTiempo = 0;
const unsigned long UMBRAL_INTERVALO = 300; // ms: si se pulsa más rápido que esto, cuenta como "rápido"
int contadorRapido = 0;
const int LIMITE_PULSACIONES = 10;
int ultimoEstado = HIGH;


void setup() {
  Serial.begin(115200);
  input_password.reserve(32); // maximum input characters is 33 (keypad)
  Wire.begin(21, 17);

  //WIFI
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, passwordWIFI);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" ¡Conectado!");
  Serial.print("Canal WiFi actual: ");
  Serial.println(WiFi.channel());

  //ESP-NOW
  esp_now_init();
  registrarPeer();
  Serial.println(myData.msg);

  pinMode(PIN_CERRADURA, OUTPUT);  //CERRADURA
  pinMode(PIN_finalCarreras, INPUT_PULLUP);  //FINAL DE CARRERAS

  //RFID
  SPI.begin();        // Iniciar bus SPI
  mfrc522.PCD_Init(); // Iniciar RC522
  Serial.println("Lector RFID listo. Acerca una tarjeta...");

  //LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();  
}

void loop() {
  
  //FINAL DE CARRERAS
  int lectura = digitalRead(PIN_finalCarreras);
  // detectar flanco: cuando pasa de HIGH a LOW (pulsado)
  if (lectura == LOW && ultimoEstado == HIGH) {  //HIGH puerta abierta; LOW puerta cerrada
    unsigned long ahora = millis();
    unsigned long intervalo = ahora - ultimoTiempo;
    ultimoTiempo = ahora;

    if (intervalo < UMBRAL_INTERVALO) {
      contadorRapido++;
      Serial.print("Pulsación rápida #");
      Serial.println(contadorRapido);
    } else {
      contadorRapido = 1; // reiniciar la cuenta, esta es la primera de una nueva secuencia
      Serial.println("Pulsación normal, contador reseteado.");
    }

    if (contadorRapido >= LIMITE_PULSACIONES) {
      Serial.println("¡¡DEMASIADAS PULSACIONES!!");
      contadorRapido = 1;
      
    }
  }
  ultimoEstado = lectura;
   
  //KEYPAD
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
    delay(2000);
    estadoN = 1; 
  }

  if (key) {
    Serial.println(key);  
    lcd.print("*");

    switch (key) {

      case '*':
        input_password = ""; // limpiar input password
        lcd.clear();
        Serial.println(password);
        estadoK = 0; 
        break;

      case '#':
        if(estadoK == 0) {
          if (password == input_password) {
            
            Serial.println("The password is correct");

            lcd.clear();
            lcd.print("La contraseña");
            lcd.setCursor(2, 1);
            lcd.print("es correcta");
            delay(2000);
            lcd.clear();

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
            lcd.clear();
            lcd.print("La constraseña");
            lcd.setCursor(2, 1);
            lcd.print("es incorrecta");
            delay(2000);
            lcd.clear();

            intentos++;
            Serial.println(intentos);
            if(intentos >= 5){
              mensaje1();
              estadoK = 3;
              tiempoDeInicio = millis();
            } else {
              estadoK = 0;
              estadoC = 0;
              Serial.println(estadoK);
              Serial.println(password);
            }

          }

          input_password = "";

        } else if(estadoK == 1){

          if(input_password == "123"){
            //ingresar nueva clave
            Serial.println("ingrese la nueva clave");

            lcd.clear();
            lcd.print("ingrese");
            lcd.setCursor(2, 1);
            lcd.print("nueva clave");

            estadoK = 2;
            Serial.println(password);
            Serial.println(estadoK);


          } else if (input_password == "456") {
            //acreditar tarjeta y asignarle un ID

            lcd.clear();
            lcd.print("Acerque tarjeta");
            lcd.setCursor(2, 1);
            lcd.print("para acreditar");
            delay(2000);
            lcd.clear();

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

                lcd.clear();
                lcd.print("Tarjeta");
                lcd.setCursor(2, 1);
                lcd.print("acreditada");
                delay(1500);
                lcd.clear();
                lcd.print("Con el ID: ");
                lcd.setCursor(2, 1);
                lcd.print(authorizedIDs[tagCount]);
                delay(2000);
                lcd.clear();

                nextID++;
                tagCount++;
                estadoK = 0;
                uid = "";
              }  

            } else {
              Serial.println("Esta tarjeta ya esta autorizada");
              lcd.clear();
              lcd.print("Tarjeta ya");
              lcd.setCursor(2, 1);
              lcd.print("autorizada");
              delay(1500);
              lcd.clear();
              estadoK = 0;
            }
            
            estadoK = 0;
            delay(1000);

          } else if(input_password == "ABC"){
            //desacreditar tarjeta 
            Serial.println("Ingrese ID de la tarjeta que desea eliminar");
            lcd.clear();
            lcd.print("ingrese ID para");
            lcd.setCursor(2, 1);
            lcd.print("eliminar tarjeta");
            delay(1500);
            lcd.clear();
            estadoK = 4;
            input_password = "";


          } else if(input_password == "BCD"){
            //mostrar ID
            Serial.println("acerque la tarjeta");
            lcd.clear();
            lcd.print("acerque la");
            lcd.setCursor(2, 1);
            lcd.print("tarjeta");
            delay(1500);
            lcd.clear();

            while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
            }  

            uid = getUID();
            Serial.println(uid);
            int BLABLA = encontrarTag(uid);
            if(BLABLA == -1) {
              Serial.println("Tarjeta sin ID asignado");

              lcd.clear();
              lcd.print("Tarjeta sin ID");
              lcd.setCursor(2, 1);
              lcd.print("asignado");
              delay(1500);
              lcd.clear();
              
              delay(5000);
              input_password = "";
              estadoK = 0;           
            } else {
              encontrarID(uid);
              Serial.print("Con el ID: ");
              Serial.println(ID);
              
              lcd.clear();
              lcd.print("Tarjeta con el ID: ");
              lcd.setCursor(2, 1);
              lcd.print(ID);

              delay(1500);
              lcd.clear();

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

          lcd.clear();
          lcd.print("Clave cambiada");
          delay(2000);
          lcd.clear();

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
      Serial.print("TARJETA CORRECTA. INGRESE");
      Serial.println(authorizedTags[index]);

      digitalWrite(PIN_CERRADURA, HIGH);
      lcd.clear();
      lcd.print("Tarjeta correcta");
      lcd.setCursor(2, 1);
      lcd.print("ingrese");
      delay(1500);
      lcd.clear();

      estadoC = 1; 
      tiempoCerradura = millis();

      uid = "";
      estadoN = 0;
    } else {
      Serial.println("TARJETA INCORRECTA");
      lcd.clear();
      lcd.print("Tarjeta");
      lcd.setCursor(2, 1);
      lcd.print("incorrecta");
      delay(1500);
      lcd.clear();
      uid = "";
      estadoN = 0;
      estadoK = 0; 
      estadoC = 0;
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


//funciones ESP-NOW
void registrarPeer() {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void mensaje1() {
  strcpy(myData.msg, "MAIL");
  esp_err_t result1 = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}
