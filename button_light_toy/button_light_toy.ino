#include <avr/sleep.h>
#include <avr/power.h>
#include "LowPower.h"
#include "array.h"

using namespace jarduino;

int Len(const int arr[]) {
  return sizeof(arr) / sizeof(int);
}

int kButtonPins[] = {2, 3};
int kLightPins[] = {9, 8, 7, 6};

// Must be volatile to be altered in InterruptCallback
volatile int woke_up_timestamp = 0;

// Durations in milliseconds for lights and sounds
int kDurations[] = {160, 320, 640, 1280};

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
  int lightPin;
  int duration;
};

Array<LightAndDuration> GetRandomLightSequence()  {
  Array<LightAndDuration> flashes;
  return flashes;
}

void ShowLights() {
  for (int i = 0; i < Len(kLightPins); ++i) {
    int lightPin = kLightPins[i];
    int duration = 500; // TODO: CHANGE IT
    int pauseBetweenNotes = duration * 1.30;
  }
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
  // Serial.begin(9600);
  for (int i = 0; i < Len(kButtonPins); ++i) {
    pinMode(kButtonPins[i], INPUT_PULLUP);
  }
  for (int i = 0; i < Len(kLightPins); ++i) {
    pinMode(kLightPins[i], OUTPUT);
  }

  InterruptCallback();  // Initialize wake up timestamp.
}



void loop() {
  if ((millis() - woke_up_timestamp) > kIdleTime) {
    DeepSleep();
    // Control will resume here when we are woken back up.
  }

    for (int i = 0; i < Len(kButtonPins); ++i) {
      // ButtonToLight(kButtonPins[i], kLightPins[i]);
    }

  
}
