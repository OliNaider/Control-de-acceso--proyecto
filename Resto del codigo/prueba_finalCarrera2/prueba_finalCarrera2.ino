const int PIN_SWITCH = 2;   // final de carreras
const int PIN_LED = 13;

unsigned long ultimoTiempo = 0;
const unsigned long UMBRAL_INTERVALO = 300; // ms: si se pulsa más rápido que esto, cuenta como "rápido"
int contadorRapido = 0;
const int LIMITE_PULSACIONES = 10;

int ultimoEstado = HIGH;

void setup() {
  pinMode(PIN_SWITCH, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  int lectura = digitalRead(PIN_SWITCH);

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
      // podés prender un LED, activar alarma, etc.
      contadorRapido = 1;
      
    }
  }

  ultimoEstado = lectura;
}

