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
String password;

void setup() {
  Serial.begin(9600);
  input_password.reserve(32); // maximum input characters is 33 (keypad)
  Wire.begin();

  lcd.init();
  lcd.clear();
  lcd.backlight();  
}

void loop() {
  password = "7890";
  char key = keypad.getKey();

  if (key) {
    
    switch (key) {

      Serial.println(key);  
      lcd.print("*");

      case '*':
        input_password = ""; // limpiar input password
        lcd.clear();
        break;

      case '#':
        if (password == input_password) {
          Serial.println("The password is correct, ACCESS GRANTED!");
          lcd.clear();
          lcd.print("The password is");
          lcd.setCursor(2, 1);
          lcd.print("correct");
          delay(500);
          lcd.clear();

          if (input_password == cambioClave) {
            Serial.println("ingrese nueva clave");
            password = key;  // Revisa esta parte, tal vez quieres otra cosa
            Serial.println(password);
          }
        } else {
          Serial.println("The password is incorrect, ACCESS DENIED!");
          lcd.clear();
          lcd.print("The password is incorrect");
          lcd.setCursor(2, 1);
          lcd.print("incorrect");
          delay(500);
          lcd.clear();
        }

        input_password = ""; // limpiar input password
        break;

      default:
        input_password += key; // agregar carácter a input password
        break;
    }
  }
}
