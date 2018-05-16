void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
}

void loop() {
  float volts =  5 * analogRead(A0) / 1024.0 * 2;
  Serial.println(volts);
  delay(500);
}
