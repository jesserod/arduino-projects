#include <avr/sleep.h>
#include <avr/power.h>
#include "LowPower.h"
#include "array.h"
#include "pitches.h"

int melody[] = {
//  NOTE_C4, NOTE_G3, NOTE_C4, NOTE_A3
  NOTE_C5, NOTE_G4, NOTE_C5, NOTE_A4
};

using namespace jarduino;

#define Len(arr) (sizeof(arr) / sizeof(int))

int kButtonPin = 3;
int kLightPins[] = {9, 8, 7, 8};
int kBottomLightPin = 6;
int kSpeakerPin = 5;
int kSwitchPin = 4;

// To avoid holding the button down spamming the song.
int button_has_been_released = true;

int LightPin(int index) {
  return kLightPins[index];
}

int NumLightPins() {
  return Len(kLightPins);
}
// Must be volatile to be altered in InterruptCallback
volatile unsigned long last_interacted_timestamp = 0;

// Durations in milliseconds for lights and sounds
int kDurations[] = {80, 160, 320};  // Quarter note, half note, whole note

// Interrupts on Arduino Pro must be pin 2 or pin 3
int kInterruptId1 = digitalPinToInterrupt(3);
//int kInterruptId2 = digitalPinToInterrupt(2);

// Idle time before device goes to sleep
unsigned long kIdleTime = 5 * 1000;  // Milliseconds

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
  const int kNumFlashes = 9;
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

  pinMode(kButtonPin, INPUT_PULLUP);
  pinMode(kSwitchPin, INPUT_PULLUP);
  
  for (int i = 0; i < NumLightPins(); ++i) {
    pinMode(kLightPins[i], OUTPUT);
  }
  pinMode(kSpeakerPin, OUTPUT);
  pinMode(kBottomLightPin, OUTPUT);

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
  bool sound_mode = digitalRead(kSwitchPin) == LOW;

  if (button_pressed) {
    last_interacted_timestamp = now;
  }

  if (sound_mode && button_pressed && button_has_been_released) {  
    FlashLights();  // Has a delay
  }
   
  if (!sound_mode && button_pressed && button_has_been_released) {
    digitalWrite(kBottomLightPin, HIGH);

    // Make each light light up one at a time to count 3 seconds
    // and make a noise the whole time.
    digitalWrite(LightPin(0), HIGH);
    delay(700);
    digitalWrite(LightPin(2), HIGH);
    delay(700);
    digitalWrite(LightPin(3), HIGH);
    Stutter(50, NOTE_C4);
    Stutter(50, NOTE_C6);
    delay(600);
    TurnLightsOff();
    digitalWrite(kBottomLightPin, LOW);
    noTone(kSpeakerPin);
  }
  digitalWrite(kBottomLightPin, LOW);
    
  button_has_been_released = !button_pressed;
  delay(50);
}
