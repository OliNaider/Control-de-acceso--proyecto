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

byte pin_rows[ROW_NUM] = {32, 33, 25, 26};
byte pin_column[COLUMN_NUM] = {27, 14, 12, 13};

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

String input_password;
const String cambioClave = "123";
String password = "7890";
int estadoK = 0;      // 0 = normal, 2 = cambiar clave, 3 = bloqueado, 4 = eliminar tarjeta
int intentos = 0;

unsigned long tiempoDeInicio = 0; 
unsigned long duracionBloqueo = 5000;
unsigned long duracionCambios = 10000; // usaré también como duración ventana especiales

// tiempo cerradura 
int estadoC = 0;
unsigned long tiempoCerradura = 0;
unsigned long duracionCerradura = 2000;

// Ventana para claves especiales (sólo se habilita luego de ingresar la contraseña correcta)
bool specialsAllowed = false;
unsigned long specialsStart = 0;
unsigned long specialsDuration = duracionCambios; // 10s (puedes ajustar)

//NFC
const int MAX_TAGS = 20; // máximo de tarjetas que se pueden guardar
String authorizedTags[MAX_TAGS];
int authorizedIDs[MAX_TAGS];
int tagCount = 0;
int nextID = 1;
String uid = "";
int estadoN = 0;
int ID = 0;

void setup() {
  Serial.begin(115200);
  input_password.reserve(32);
  Wire.begin();

  pinMode(PIN_CERRADURA, OUTPUT);
  digitalWrite(PIN_CERRADURA, LOW); // cerradura por defecto cerrada

  //RFID
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Lector RFID listo. Acerca una tarjeta...");
}

void loop() {
  char key = keypad.getKey();

  // --- manejo bloqueo por intentos ---
  if (estadoK == 3) {
    if ((millis() - tiempoDeInicio) >= duracionBloqueo) {
      estadoK = 0;
      intentos = 0;
      Serial.println("sistema desbloqueado");
    } else {
      Serial.println("SISTEMA BLOQUEADO");
    }
    return;
  }

  // --- expiración ventana especiales ---
  if (specialsAllowed && (millis() - specialsStart > specialsDuration)) {
    specialsAllowed = false;
    Serial.println("Ventana de claves especiales expiró.");
  }

  // --- cerrar cerradura tras duracionCerradura ---
  if (estadoC == 1 && (millis() - tiempoCerradura > duracionCerradura)) {
    Serial.println("cerradura cerrada");
    digitalWrite(PIN_CERRADURA, LOW);
    estadoC = 0;
  }

  // --- lectura RFID (sigue igual, salvo que no la usamos si no corresponde) ---
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    uid = getUID();
    Serial.print("UID leído: ");
    Serial.println(uid);
    delay(1000);
    estadoN = 1;
  }

  if (key) {
    Serial.println(key);

    switch (key) {
      case '*':
        input_password = "";
        Serial.println("Input limpiado");
        break;

      case '#':
        // Estado normal: puede ser ingreso de contraseña principal
        if (estadoK == 0) {
          // 1) si coincide con la contraseña principal -> abrir cerradura y habilitar ventana especiales
          if (password == input_password) {
            Serial.println("The password is correct, ACCESS GRANTED!");
            digitalWrite(PIN_CERRADURA, HIGH); // abrir
            intentos = 0;
            tiempoCerradura = millis();
            estadoC = 1;

            // habilitar ventana de claves especiales
            specialsAllowed = true;
            specialsStart = millis();
            Serial.println("Ventana de claves especiales habilitada.");

            // no cambiamos estadoK: seguimos en estado normal
          } else if (specialsAllowed) {
            Serial.println("Ventana especiales activa. Comprobando clave especial: " + input_password);

            if (input_password == "123") {
              Serial.println("ingrese la nueva clave");
              estadoK = 2; // modo cambio de clave
              // dejamos specialsAllowed activo (si querés que se apague tras usar una especial, setear a false aquí)
            } else if (input_password == "456") {
              Serial.println("Ingresar tarjeta para acreditar (acérquela)...");
              // buscar tarjeta y almacenarla (bloqueante como en tu código original)
              while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
                // espera activa hasta que acerques tarjeta
              }
              uid = getUID();
              Serial.print("UID leído: ");
              Serial.println(uid);
              int BLABLA = encontrarTag(uid);
              if (BLABLA == -1) {
                if (tagCount < MAX_TAGS) {
                  Serial.println("autorizando tarjeta");
                  authorizedTags[tagCount] = uid;
                  authorizedIDs[tagCount] = nextID;
                  Serial.print("tarjeta autorizada: ");
                  Serial.println(authorizedTags[tagCount]);
                  Serial.print("con el ID: ");
                  Serial.println(authorizedIDs[tagCount]);
                  nextID++;
                  tagCount++;
                  uid = "";
                } else {
                  Serial.println("Capacidad máxima de tarjetas alcanzada.");
                }
              } else {
                Serial.println("Esta tarjeta ya esta autorizada");
              }
              estadoK = 0;
              delay(1000);
            } else if (input_password == "ABC") {
              Serial.println("Ingrese ID de la tarjeta que desea eliminar");
              estadoK = 4; // modo eliminar ID
              input_password = "";
            } else if (input_password == "BCD") {
              Serial.println("acerque la tarjeta para mostrar ID");
              while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
              }
              uid = getUID();
              Serial.println(uid);
              int BLABLA = encontrarTag(uid);
              if (BLABLA == -1) {
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
            } else {
              // clave incorrecta dentro de ventana de especiales
              Serial.println("Clave especial no reconocida.");
            }
          } else {
            Serial.println("The password is incorrect, ACCESS DENIED!");
            intentos++;
            Serial.println(intentos);
            if (intentos >= 5) {
              estadoK = 3;
              tiempoDeInicio = millis();
            } else {
              estadoK = 0;
            }
          }

          input_password = "";
        }
        // estadoK == 2 -> estamos en modo cambiar clave y # confirma la nueva clave
        else if (estadoK == 2) {
          password = input_password;
          Serial.println("clave cambiada: " + password);
          estadoK = 0;
          input_password = "";
        }
        // estadoK == 4 -> eliminar tag por ID
        else if (estadoK == 4) {
          int eliminarID = input_password.toInt();
          eliminarTag(eliminarID);
          estadoK = 0;
          input_password = "";
        }
        break;

      default:
        // caracteres normales
        input_password += key;
        break;
    } // switch
  } // if key

  // --- manejo RFID cuando una tarjeta es leída en modo normal (entrada directa por tarjeta) ---
  if (estadoN == 1) {
    int index = encontrarTag(uid);
    if (index != -1) {
      Serial.println("TARJETA CORRECTA. INGRESE!!!!");
      Serial.println(authorizedTags[index]);
      digitalWrite(PIN_CERRADURA, HIGH);
      tiempoCerradura = millis();
      estadoC = 1;

      // cuelga la ventana de especiales si querés que la tarjeta también autorice la ventana:
      // specialsAllowed = true; specialsStart = millis();

      uid = "";
      estadoN = 0;
    } else {
      Serial.println("TARJETA INCORRECTA");
      uid = "";
      estadoN = 0;
    }
  }
}

// ------------------------------------------------------------------
// FUNCIONES RFID (sin cambios significativos)
String getUID() {
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();
  return uidString;
}

int encontrarTag(String tag) {
  for (int i = 0; i < tagCount; i++) {
    if (authorizedTags[i] == tag) {
      return i;
    }
  }
  return -1;
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
  for (int i = 0; i < tagCount; i++) {
    if (authorizedIDs[i] == ID) {
      for (int j = i; j < tagCount - 1; j++) {
        authorizedTags[j] = authorizedTags[j + 1];
        authorizedIDs[j] = authorizedIDs[j + 1];
      }
      tagCount--;
      Serial.println("Eliminacion completada");
      return;
    }
  }
  Serial.println("ID no encontrado para eliminar");
}

void encontrarID(String tag) {
  for (int i = 0; i < tagCount; i++) {
    if (authorizedTags[i] == tag) {
      ID = authorizedIDs[i];
      return;
    }
  }
  ID = 0; // no encontrado
}
