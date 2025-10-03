// Conexión:
// COM -> GND
// NO  -> pin 2
// Usa el LED integrado (Arduino UNO: 13, ESP32: 2 normalmente)

const int PIN_SWITCH = 2;    
const int PIN_LED = 13;   // En ESP32 puede ser el 2

int contador = 0; 
unsigned long tiempoDeInicio = 0; 
unsigned long duracion = 1000;


void setup() {
  pinMode(PIN_SWITCH, INPUT_PULLUP); // pullup interno
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200);
}

void loop() {

  if (digitalRead(PIN_SWITCH) == LOW) { 
    // Pulsado
    digitalWrite(PIN_LED, HIGH);
    Serial.println("Pulsado");


  } else {
    // No pulsado
    digitalWrite(PIN_LED, LOW);
    Serial.println("No pulsado");
  }
  delay(100);

  if(contador > 10){
    Serial.println("Se pulsaron demasiadas veces");
    contador = 0;
  }

  (millis() - tiempoDeInicio) >= duracionBloqueo;
}

