#include <avr/sleep.h>
#include <avr/power.h>
#include "LowPower.h"
#include "array.h"
#include "pitches.h"
#include <Adafruit_NeoPixel.h>


int melody[] = {
//  NOTE_C4, NOTE_G3, NOTE_C4, NOTE_A3
  NOTE_C5, NOTE_G4, NOTE_C5, NOTE_A4
};

using namespace jarduino;

#define Len(arr) (sizeof(arr) / sizeof(int))

int kButtonPin = 2;
int kPixelPin = 5;
int kLedPin = 6;
int kSpeakerPin = 7;


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, kPixelPin, NEO_GRB + NEO_KHZ800);
// int kSwitchPin = 4;

// To avoid holding the button down spamming the song.
int button_has_been_released = true;



// Must be volatile to be altered in InterruptCallback
volatile unsigned long last_interacted_timestamp = 0;

// Durations in milliseconds for lights and sounds
int kDurations[] = {80, 160, 320};  // Quarter note, half note, whole note


int kInterruptId1 = digitalPinToInterrupt(kButtonPin);

// Idle time before device goes to sleep
unsigned long kIdleTime = 5 * 1000;  // Milliseconds

// Captures a light pin and how long we want to have it on.
struct LightAndDuration {
  int light_pin;
  int duration;
  int light_index;
};


void TurnLightsOff() {
  pixels.setPixelColor(0, pixels.Color(0,0,0)); // Moderately bright green color.
  pixels.show(); // This sends the updated pixel color to the hardware.
}


void InterruptCallback()
{
  /* This will bring us back from sleep. */

  /* We detach the interrupt to stop it from
     continuously firing while the interrupt pin
     is low.
  */
  detachInterrupt(kInterruptId1);
  //detachInterrupt(kInterruptId2);
}

void DeepSleep() {
  // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(kInterruptId1, InterruptCallback, LOW);
 // attachInterrupt(kInterruptId2, InterruptCallback, LOW);

  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}


////////////////////////////////
void setup() {
  Serial.begin(9600);
  delay(50);
  Serial.println("setup()");
  delay(50);
  button_has_been_released = true;
  tone(kSpeakerPin, NOTE_C5, 1000);

  pinMode(kButtonPin, INPUT_PULLUP);
  //pinMode(kSwitchPin, INPUT_PULLUP);

  pixels.begin(); // This initializes the NeoPixel library.

  pinMode(kSpeakerPin, OUTPUT);
  pinMode(kLedPin, OUTPUT);

  last_interacted_timestamp = millis();
  // InterruptCallback();  // Initialize wake up timestamp.
  srand(analogRead(A4));  // Create random seed by reading noise from pin
}

void Stutter(int duration, int note) {
  int silence = 30;
  int sound = 20;
  for (int elapsed = 0; elapsed < duration; elapsed += silence) {
    tone(kSpeakerPin, note, sound);
    //noTone(kSpeakerPin);
    delay(silence);
  }
}

void loop() {
  unsigned long now = millis();
  if ((now - last_interacted_timestamp) > kIdleTime) {
    Serial.print("DEEP SLEEP ");
    Serial.print(now);
    Serial.print(" ");
    Serial.print(last_interacted_timestamp);
    Serial.print(" ");
    Serial.print(now - last_interacted_timestamp);
    Serial.print(" ");
    Serial.print(kIdleTime);
    Serial.println();
    delay(100);
    DeepSleep();
    last_interacted_timestamp = now;
    Serial.println("Woke up!");
    return;
    // Control will resume here when we are woken back up.
  }
  bool button_pressed = digitalRead(kButtonPin) == LOW;
  // bool sound_mode = digitalRead(kSwitchPin) == LOW;

  if (button_pressed) {
    
    last_interacted_timestamp = now;
  }
  
  if (button_pressed && button_has_been_released) {
    Serial.println("Pressed!");
    digitalWrite(kLedPin, HIGH);

    // Make each light light up one at a time to count 3 seconds
    // and make a noise the whole time.
    pixels.setPixelColor(0, pixels.Color(150,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.

    delay(700);
    pixels.setPixelColor(0, pixels.Color(0,0,150));
    pixels.show(); // This sends the updated pixel color to the hardware.

    delay(700);
    pixels.setPixelColor(0, pixels.Color(0,150,0));
    pixels.show(); // This sends the updated pixel color to the hardware.

    Stutter(50, NOTE_C4);
    Stutter(50, NOTE_C6);
    delay(600);
    TurnLightsOff();
    digitalWrite(kLedPin, LOW);
    noTone(kSpeakerPin);
  }
  digitalWrite(kLedPin, LOW);
    
  button_has_been_released = !button_pressed;
  delay(50);
}
