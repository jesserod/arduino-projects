#include <avr/sleep.h>
#include <avr/power.h>
#include "LowPower.h"
#include "array.h"

using namespace jarduino;

#define Len(arr) (sizeof(arr) / sizeof(int))

int kButtonPins[] = {2, 3};
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
volatile int woke_up_timestamp = 0;

// Durations in milliseconds for lights and sounds
int kDurations[] = {80, 160, 320, 640};

// Interrupts on Arduino Pro must be pin 2 or pin 3
int kInterruptId1 = digitalPinToInterrupt(2);
int kInterruptId2 = digitalPinToInterrupt(3);

// Idle time before device goes to sleep
int kIdleTime = 5000;  // Milliseconds

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
};

Array<LightAndDuration> GetRandomLightSequence()  {
  const int kNumFlashes = 4;
  Array<LightAndDuration> flashes(kNumFlashes);
  for (int i = 0; i < kNumFlashes; ++i) {
    flashes.Get(i).light_pin = LightPin(random(NumLightPins()));
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
    Serial.println(flash.duration);
    delay(flash.duration);
    TurnLightsOff();
    int pauseBetweenFlashes = flash.duration * 1.30;
    delay(pauseBetweenFlashes);
  }
  Serial.println("END FlashLights()");
}

void InterruptCallback()
{
   /* This will bring us back from sleep. */
  
  /* We detach the interrupt to stop it from 
   * continuously firing while the interrupt pin
   * is low.
   */
  detachInterrupt(kInterruptId1);
  detachInterrupt(kInterruptId2);
  woke_up_timestamp = millis();
}

void DeepSleep() {
  // Allow wake up pin to trigger interrupt on low.
      attachInterrupt(kInterruptId1, InterruptCallback, LOW);
      attachInterrupt(kInterruptId2, InterruptCallback, LOW);
    
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
}


////////////////////////////////
void setup() {
  Serial.begin(9600);
  for (int i = 0; i < Len(kButtonPins); ++i) {
    pinMode(kButtonPins[i], INPUT_PULLUP);
  }
  for (int i = 0; i < NumLightPins(); ++i) {
    pinMode(kLightPins[i], OUTPUT);
  }

  // InterruptCallback();  // Initialize wake up timestamp.
}



void loop() {
  /*
  if ((millis() - woke_up_timestamp) > kIdleTime) {
    DeepSleep();
    // Control will resume here when we are woken back up.
  }
  */
  /*
    if (digitalRead(ButtonPin(0)) == LOW) {
      Serial.println("Hello");
      digitalWrite(LightPin(0), HIGH);
    }
    */

  /*
  digitalWrite(LightPin(0), HIGH);
  delay(250);
  digitalWrite(LightPin(0), LOW);
  digitalWrite(LightPin(1), HIGH);
  delay(250);
  digitalWrite(LightPin(1), LOW);
  digitalWrite(LightPin(2), HIGH);
  delay(250);
  digitalWrite(LightPin(2), LOW);
  digitalWrite(LightPin(3), HIGH);
  delay(250);
  digitalWrite(LightPin(3), LOW);
  */

  for (int i = 0; i < NumButtonPins(); ++i) {
    if (digitalRead(ButtonPin(i)) == LOW) {
      //digitalWrite(LightPin(0), HIGH);
      //delay(1000);
      FlashLights();
      break;
    }
  }
}
