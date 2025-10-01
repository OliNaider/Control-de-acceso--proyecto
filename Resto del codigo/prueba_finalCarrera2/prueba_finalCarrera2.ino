const int PIN_SWITCH = 2;    
const int PIN_LED = 13;   // En ESP32 puede ser el 2

enum Estado {
  ESPERANDO_PULSACION,
  PULSADO,
  BLOQUEADO
};

Estado estadoActual = ESPERANDO_PULSACION;

int contador = 0; 
unsigned long tiempoDeInicio = 0; 
unsigned long duracionBloqueo = 5000;  // 5 segundos de bloqueo

void setup() {
  pinMode(PIN_SWITCH, INPUT_PULLUP); // pullup interno
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  switch (estadoActual) {
    case ESPERANDO_PULSACION:
      if (digitalRead(PIN_SWITCH) == LOW) {
        estadoActual = PULSADO;
      } else {
        digitalWrite(PIN_LED, LOW);
        Serial.println("No pulsado");
      }
      break;

    case PULSADO:
      digitalWrite(PIN_LED, HIGH);
      Serial.println("Pulsado");
      contador++;

      if (contador > 10) {
        Serial.println("Se pulsaron demasiadas veces");
        estadoActual = BLOQUEADO;
        tiempoDeInicio = millis();
        digitalWrite(PIN_LED, LOW);  // Apagar el LED durante el bloqueo
      } else {
        estadoActual = ESPERANDO_PULSACION; // Vuelve a esperar nueva pulsación
      }
      break;

    case BLOQUEADO:
      if (millis() - tiempoDeInicio >= duracionBloqueo) {
        Serial.println("Fin del bloqueo");
        contador = 0;
        estadoActual = ESPERANDO_PULSACION;
      } else {
        Serial.println("Bloqueado...");
        digitalWrite(PIN_LED, LOW);
      }
      break;
  }

  delay(100);  // Pequeña pausa para evitar lecturas erráticas
}
