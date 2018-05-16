void setup() {
  Serial.begin(9600);
  for (int pin = 0; pin < 30; pin++) {
    pinMode(pin, INPUT_PULLUP);
  }
}

void PrintPin(int pin) {
  Serial.print("[");
  Serial.print(pin);
  Serial.print("]=");
  Serial.print(digitalRead(pin));
  Serial.print(" ");
}

void PrintAllPinStates() {
  for (int pin = 0; pin < 10; pin++) {
    Serial.print(" ");
    PrintPin(pin);
  }
  Serial.print("\n");
  for (int pin = 10; pin < 20; pin++) {
    PrintPin(pin);
  }
   Serial.print("\n");
  for (int pin = 20; pin < 30; pin++) {
    PrintPin(pin);
  }
  Serial.println("\n");
}


void loop() {
  for (int i = 0; i < 20; i++) {
    Serial.println();
  }

  PrintAllPinStates();
  delay(500);
}
