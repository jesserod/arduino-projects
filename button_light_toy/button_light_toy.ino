#include <avr/sleep.h>
#include <avr/power.h>
#include "LowPower.h"
#include "array.h"
#include "pitches.h"

int kSpeakerPin = 4;

int melody[] = {
//  NOTE_C4, NOTE_G3, NOTE_C4, NOTE_A3
  NOTE_C5, NOTE_G4, NOTE_C5, NOTE_A4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
// int noteDurations[] = {
// 4, 8, 8, 4, 4, 4, 4, 4
// };


using namespace jarduino;

#define Len(arr) (sizeof(arr) / sizeof(int))

int kButtonPins[] = {2 /*, 3*/};
int kLightPins[] = {9, 8, 7, 6};

int LightPin(int index) {
  return kLightPins[index];
}

int NumLightPins() {
  return Len(kLightPins);
}

int ButtonPin(int index) {
  return kButtonPins[index];
}

int NumButtonPins() {
  return Len(kButtonPins);
}

// Must be volatile to be altered in InterruptCallback
volatile int last_interacted_timestamp = 0;

// Durations in milliseconds for lights and sounds
int kDurations[] = {80, 160, 320};

// Interrupts on Arduino Pro must be pin 2 or pin 3
int kInterruptId1 = digitalPinToInterrupt(2);
//int kInterruptId2 = digitalPinToInterrupt(3);

// Idle time before device goes to sleep
int kIdleTime = 120 * 1000;  // Milliseconds

void SetLightState(int lightPin, bool lightIsOn) {
  if (lightIsOn) {
    digitalWrite(lightPin, HIGH);
  } else {
    digitalWrite(lightPin, LOW);
  }
}

// Captures a light pin and how long we want to have it on.
struct LightAndDuration {
  int light_pin;
  int duration;
  int light_index;
};

Array<LightAndDuration> GetRandomLightSequence()  {
  const int kNumFlashes = 6;
  Array<LightAndDuration> flashes(kNumFlashes);
  for (int i = 0; i < kNumFlashes; ++i) {
    int light_index = random(NumLightPins());
    flashes.Get(i).light_index = light_index;
    flashes.Get(i).light_pin = LightPin(light_index);
    flashes.Get(i).duration = kDurations[random(Len(kDurations))];

  }
  return flashes;
}

// Turn this light pin on, and all others off.
void TurnLightOn(int light_pin) {
  for (int p = 0; p < NumLightPins(); ++p) {
    if (LightPin(p) == light_pin) {
      digitalWrite(LightPin(p), HIGH);
    } else {
      digitalWrite(LightPin(p), LOW);
    }
  }
}

void TurnLightsOff() {
  for (int p = 0; p < NumLightPins(); ++p) {
    digitalWrite(LightPin(p), LOW);
  }
}

void FlashLights() {
  Serial.println("FlashLights()");
  Array<LightAndDuration> flashes = GetRandomLightSequence();

  for (int f = 0; f < flashes.Size(); ++f) {
    auto& flash = flashes.Get(f);
    Serial.print("Turning on light pin ");
    Serial.println(flash.light_pin);
    TurnLightOn(flash.light_pin);
    Serial.print("Sleeping for ");
    tone(kSpeakerPin, melody[flash.light_index], flash.duration);
    Serial.println(flash.duration);
    delay(flash.duration);
    TurnLightsOff();
    noTone(kSpeakerPin);
    //  int pauseBetweenFlashes = max(flash.duration * 1.30, 100);
    int pauseBetweenFlashes = 50;
    delay(pauseBetweenFlashes);
  }
  Serial.println("END FlashLights()");
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
  last_interacted_timestamp = millis();
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
  for (int i = 0; i < Len(kButtonPins); ++i) {
    pinMode(kButtonPins[i], INPUT_PULLUP);
  }
  for (int i = 0; i < NumLightPins(); ++i) {
    pinMode(kLightPins[i], OUTPUT);
  }

  last_interacted_timestamp = millis();
  // InterruptCallback();  // Initialize wake up timestamp.
  srand(analogRead(A4));
}



void loop() {

  if ((millis() - last_interacted_timestamp) > kIdleTime) {
    Serial.println("DEEP SLEEP");
    delay(100);
    DeepSleep();
    Serial.println("Woke up!");
    // Control will resume here when we are woken back up.
  }



  for (int i = 0; i < NumButtonPins(); ++i) {
    if (digitalRead(ButtonPin(i)) == LOW) {
      FlashLights();
      last_interacted_timestamp = millis();
      break;
    }
  }

  delay(50);
}
