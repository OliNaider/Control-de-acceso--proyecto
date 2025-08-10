#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // three columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM] = {12, 11, 10, 9}; // GPIO18, GPIO5, GPIO17, GPIO16 connect to the row pins
byte pin_column[COLUMN_NUM] = {8, 7, 6, 5};  // GPIO4, GPIO0, GPIO2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
 
String input_password;
const String cambioClave = "123";
String password = "7890";
int estado = 0;
int intentos = 0;

unsigned long tiempoDeInicio = 0; 
unsigned long duracionBloqueo = 5000;

void setup() {
  Serial.begin(9600);
  input_password.reserve(32); // maximum input characters is 33 (keypad)
  Wire.begin();

  lcd.init();
  lcd.clear();
  lcd.backlight();  
}

void loop() {
  char key = keypad.getKey();

  if(estado == 3) {
    if((millis() - tiempoDeInicio) >= duracionBloqueo) {
      estado = 0;
      intentos = 0;
      Serial.println("sistema desbloqueado");
    } else {
      Serial.println("SISTEMA BLOQUEADO");
    }
    return;
  }

  if (key) {
    
    Serial.println(key);  
    lcd.print("*");

    switch (key) {

      case '*':
        input_password = ""; // limpiar input password
        lcd.clear();
        Serial.println(password);
        break;

      case '#':
        if(estado == 0) {
          if (password == input_password) {
            Serial.println("The password is correct, ACCESS GRANTED!");
            lcd.clear();
            lcd.print("The password is");
            lcd.setCursor(2, 1);
            lcd.print("correct");
            delay(500);
            lcd.clear();

            estado = 1;
            intentos = 0; 

            Serial.println(password);
            Serial.println(estado);

          } else {
            Serial.println("The password is incorrect, ACCESS DENIED!");
            lcd.clear();
            lcd.print("The password is incorrect");
            lcd.setCursor(2, 1);
            lcd.print("incorrect");
            delay(500);
            lcd.clear();

            intentos++;
            Serial.println(intentos);
            if(intentos >= 5){
              estado = 3;
              tiempoDeInicio = millis();
            } else {
              estado = 0;
              Serial.println(estado);
              Serial.println(password);
            }

          }

          input_password = "";

        } else if(estado == 1){
          if(input_password == "123"){
            Serial.println("ingrese la nueva clave");
            estado = 2;
            Serial.println(password);
            Serial.println(estado);
          } else {
            estado = 0;
            Serial.println("no se ha cambiado la clave");
            Serial.println(password);
            Serial.println(estado);
          }

          input_password = "";

        }else if(estado == 2) {
          password = input_password;
          Serial.println(password);
          Serial.println("clave cambiada");
          Serial.println(password);
          estado = 0;
          Serial.println(estado);

          input_password = "";
        }

        break;

      default:
        input_password += key; // agregar carácter a input password
        break;
    } 
  }
}

