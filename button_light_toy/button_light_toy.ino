/*
 * Button light toy with 2 modes: song mode and scanner mode.
 * - 3 lights on top and 1 light on the bottom.
 * - 1 button to activate toy, 1 switch to change modes.
 * 
 * Scanner mode:
 * - Upon button press, the light on the bottom immediately
 * lights up as if it's a scanner (e.g. like at a cash register).
 * - Sound will also begin.
 * - Additionally, the 3 lights at the top light up one by one
 * and when the third light is finished, a "complete" sound
 * is made.
 * - Lights on top and bottom will remain on if the button is
 * held, otherwise, the lights will turn off after the completion
 * sound.
 * 
 * Song mode:
 * - Pressing the button will play a short random tune with
 * lights on top appearing simultaneously with each tone.
 *   
 * Power saving:
 * - After a few seconds of inactivity, the toy enters deep
 * sleep to save power even when the toy is not powered on.
 * - Should immediately awake from deep sleep upon button
 * press without delay.
 * - Requires a ATmega328p processor to perform deep sleep
 * functionality.  This canallow allow the toy to maintain
 * battery life over weeks or months even when the power is
 * not manually turned off. To achieve this, ideally, you can
 * cut the on-board LED of the Arduino, for example?
 * http://www.home-automation-community.com/arduino-low-power-how-to-run-atmega328p-for-a-year-on-coin-cell-battery/
 *   
 * Verified compilation for Arduino Pro Mini / ATmega328p 8mhz
 */


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
int button_was_unpressed = true;

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

void TurnTopLightsOff() {
  for (int p = 0; p < NumLightPins(); ++p) {
    digitalWrite(LightPin(p), LOW);
  }
}

void PlaySongAndFlashLights() {
  Serial.println("PlaySongAndFlashLights()");
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
    TurnTopLightsOff();
    noTone(kSpeakerPin);
    //  int pauseBetweenFlashes = max(flash.duration * 1.30, 100);
    int pauseBetweenFlashes = 50;
    delay(pauseBetweenFlashes);
  }
  Serial.println("END PlaySongAndFlashLights()");
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
  button_was_unpressed = true;

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

void PureSound(int duration, int note) {
  //StaccatoTone(duration, note, 50 /* off_millis */, 75 /* on_millis */);
  tone(kSpeakerPin, note, duration);
  // delay(duration);
  // Don't use noTone if we want sounds to run together.
  //noTone(kSpeakerPin);
}

void GrittySound(int duration, int note) {
  StaccatoTone(duration, note, 30 /* off_millis */, 20 /* on_millis */);
}

// Turns speaker on/off in rapid succession so the total wall
// time elapsed is duration.  When the speaker is on, it will
// emit the specified note.
void StaccatoTone(int duration, int note, int sound_off_millis, int sound_on_millis) {
  for (int elapsed = 0; elapsed < duration; elapsed += sound_off_millis) {
    tone(kSpeakerPin, note, sound_on_millis);
    //noTone(kSpeakerPin);
    delay(sound_off_millis);
  }
  noTone(kSpeakerPin);
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
  bool button_is_down = digitalRead(kButtonPin) == LOW;
  bool song_mode = digitalRead(kSwitchPin) == LOW;
  bool scanning_mode = !song_mode;
  bool button_just_pressed = button_is_down && button_was_unpressed;
  
  if (button_is_down) {
    last_interacted_timestamp = now;
  }

  if (song_mode && button_just_pressed) {  
    PlaySongAndFlashLights();  // Has a built-in delay
  }
   
  if (scanning_mode && button_just_pressed) {
    digitalWrite(kBottomLightPin, HIGH);

    // Make each light light up one at a time to count 3 seconds
    // and make a noise the whole time.
    digitalWrite(LightPin(0), HIGH);
    PureSound(100, NOTE_G6);  // doesn't delay
    delay(100);
    digitalWrite(LightPin(2), HIGH);
    PureSound(100, NOTE_E6);  // doesn't delay
    delay(100);
    digitalWrite(LightPin(3), HIGH);
    GrittySound(100, NOTE_C6);
    GrittySound(100, NOTE_C8);
    delay(500);
    noTone(kSpeakerPin);
  }

  // Ensure all lights are off, unless we are in scanning mode
  // and we're still holding the button down.
  if (!(scanning_mode && button_is_down)) {
    TurnTopLightsOff();
    digitalWrite(kBottomLightPin, LOW);
  }

  // To avoid button-holds to keep triggering lights/sounds over and over.
  button_was_unpressed = !button_is_down;
  delay(50);
}
