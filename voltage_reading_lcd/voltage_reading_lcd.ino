// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  analogReference(EXTERNAL);  // AREF is connected to 3.3V pin
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  //Serial.begin(9600);
  pinMode(A0, INPUT);
  lcd.setCursor(0, 0);
  lcd.print("Voltage:        ");
  
}

void loop() {
  float volts =  3.3 * analogRead(A0) / 1024.0 * 2;
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(String(volts) + "v");
  
  //Serial.println(volts);
  delay(500);
}
