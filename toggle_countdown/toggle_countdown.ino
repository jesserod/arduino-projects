#include <task_timer.h>
#include <input_tracker.h>
#include <flat_long_map.h>
#include <SevSeg.h>

const int SWITCH_ON = LOW;
const int SWITCH_OFF = HIGH;

SevSeg sevseg;
//TaskTimer task_timer(A2);
InputUtil input_util(A2);

void setup()
{
   byte numDigits = 4;
  // Pinout for SH5461AS. dp = decimal, # = digit number, letters = segments
  // Top Row
  //1,a,f,2,3,b
  //Bottom Row
  //e,d,dp,c,g,4
  
  // Arduino pins connected to digits from 1..N
  // These pins should have resistors!
  byte digitPins[] = {5, 4, 3, 2};
  // Arduino pins for segments A, ..., G, Decimal
  byte segmentPins[] = {13,11,9,7,6,12,10,8};
   
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default=false. Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(1);

  input_util.RegisterInputPin(A2, INPUT_PULLUP);
  
  //task_timer.RegisterTask(A2);
  //task_timer.Update();

  Serial.begin(9600);
}

void loop()
{
  static unsigned long timer = millis();
  static float countdownFromSeconds = 11;
  static int deciSeconds = countdownFromSeconds * 10;
  
  if (millis() - timer >= 100) {
    timer += 100;
    deciSeconds--; // 100 milliSeconds is equal to 1 deciSecond
    
    if (deciSeconds < 0) { // Reset to 0 after counting for 1000 seconds.
      deciSeconds = 0;
    }
    const int decimals = -1;
    sevseg.setNumber(deciSeconds / 10, -1);
  }
  if (deciSeconds == 0) {
    sevseg.blank();
  }
  sevseg.refreshDisplay(); // Must run repeatedly with no delays (see updateWithDelays)

  // Serial.println(digitalRead(A2));
  
  if (input_util.IsChangedTo(A2, SWITCH_ON)) {
    deciSeconds = countdownFromSeconds * 10;
  }
  //task_timer.Update();
  input_util.Update();
  
  // delay(10);
}
