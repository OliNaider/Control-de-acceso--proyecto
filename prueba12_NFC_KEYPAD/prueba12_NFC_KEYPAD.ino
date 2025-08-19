#include <Keypad.h> //keypad
#include <Wire.h>

//NFC
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

//#include <LiquidCrystal_I2C.h>
//LiquidCrystal_I2C lcd(0x27, 16, 2);

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
 
//KEYPAD 
String input_password;
const String cambioClave = "123";
String password = "7890";
int estadoK = 0;
int intentos = 0;

unsigned long tiempoDeInicio = 0; 
unsigned long duracionBloqueo = 5000;

//NFC
const int MAX_TAGS = 20; // máximo de tarjetas que se pueden guardar
String authorizedTags[MAX_TAGS]; // array de UIDs
int tagIDs[MAX_TAGS];            // array paralelo de IDs
int tagCount = 0;                // cuántas tarjetas hay guardadas
int nextID = 1;                  // próximo ID a asignar
String tagId = "";

void setup() {
  Serial.begin(115200);
  input_password.reserve(32); // maximum input characters is 33 (keypad)
  Wire.begin();

  //lcd.init();
  //lcd.clear();
  //lcd.backlight();  
}

void loop() {

  char key = keypad.getKey();
  readNFC();

  int BLABLA = encontrarTag(tagId);

  if(BLABLA == 1){
    //la tarjeta esta autorizada 
  } else if (BLABLA == -1) {
    //la trjeta no esta autorizada 
    //estadoN = 1 --> que te mande a auntorizarla (tenes que poner un codigo especifico) o que sea incorrecta 
  }

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

  if (key) {
    
    Serial.println(key);  
    //lcd.print("*");

    switch (key) {

      case '*':
        input_password = ""; // limpiar input password
        //lcd.clear();
        Serial.println(password);
        break;

      case '#':
        if(estadoK == 0) {
          if (password == input_password) { 
            Serial.println("The password is correct, ACCESS GRANTED!");
            //lcd.clear();
            //lcd.print("The password is");
            //lcd.setCursor(2, 1);
            //lcd.print("correct");
            //delay(500);
            //lcd.clear();

            estadoK = 1;
            intentos = 0; 

            Serial.println(password);
            Serial.println(estadoK);

          } else {
            Serial.println("The password is incorrect, ACCESS DENIED!");
            //lcd.clear();
            //lcd.print("The password is incorrect");
            //lcd.setCursor(2, 1);
            //lcd.print("incorrect");
            //delay(500);
            //lcd.clear();

            intentos++;
            Serial.println(intentos);
            if(intentos >= 5){
              estadoK = 3;
              tiempoDeInicio = millis();
            } else {
              estadoK = 0;
              Serial.println(estado);
              Serial.println(password);
            }

          }

          input_password = "";

        } else if(estado == 1){
          if(input_password == "123"){
            Serial.println("ingrese la nueva clave");
            estadoK = 2;
            Serial.println(password);
            Serial.println(estadoK);

          } else if(input_password == "456"){
            Serial.println("ingrese nueva tarjeta");

          } else {
            estado = 0;
            Serial.println("no se ha cambiado la clave");
            Serial.println(password);
            Serial.println(estadoK);
          }

          input_password = "";

        }else if(estado == 2) {
          password = input_password;
          Serial.println(password);
          Serial.println("clave cambiada");
          Serial.println(password);
          estadoK = 0;
          Serial.println(estadoK);

          input_password = "";
        }

        break;

      default:
        input_password += key; // agregar carácter a input password
        break;
    } 
  }
}


void readNFC() 
{
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    tag.print();
    tagId = tag.getUidString();
    tagId.toUpperCase();
  } 
}  

int econtrarTag(String tag) {
  for(int i = 0; i< tagCount; i++){
    if(authorizedTags[i] == tag) {
      return 1;
    } else{
      return -1;
    }
  }
}
