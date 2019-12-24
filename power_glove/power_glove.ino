#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/sleep.h>
  #include <avr/power.h>
  #include "LowPower.h"
#endif



// Only interrupts on pins 2 and 3 are supported.
#define SLEEP_PIN 3
#define INTERRUPT_ID digitalPinToInterrupt(SLEEP_PIN)
#define IDLE_TIME_BEFORE_SLEEP 15000

#define SENSOR_PIN SLEEP_PIN
#define PIXEL_INDEX 0
#define PIXEL_PIN 9


#define LIGHT_ON_TIME 3000
#define RAINBOW_TIME 1000
#define STEPS_PER_RAINBOW 50

// Brightness can range from 0 to 255
#define BRIGHTNESS 100
const int DELAY_FOR_COLOR_CHANGE = RAINBOW_TIME / STEPS_PER_RAINBOW;
const int RGB_INCREMENT = (BRIGHTNESS*3) / STEPS_PER_RAINBOW;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
int rgbColour[3] = {BRIGHTNESS, 0, 0};  // Must have only one color at max and others at 0
int current_color = 0;  // Must start as the index of the 255 color.

// Must be volatile in case we alter in WakeUpCallback
volatile unsigned long last_interacted_timestamp = 0;

#ifdef __AVR__

  void WakeUpCallback()
  {
    /* This will bring us back from sleep. */
  
    /* We detach the interrupt to stop it from
       continuously firing while the interrupt pin
       is low.
    */
    detachInterrupt(INTERRUPT_ID);
  }
  
  void DeepSleep() {
    // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(INTERRUPT_ID, WakeUpCallback, LOW);
  
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  }

#else

  // TODO:  If using Adafruit TPL5110, send a signal to the DONE pin.
  void DeepSleep() {}

#endif


void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pixels.begin(); // This initializes the NeoPixel library
  Serial.begin(9600);
  last_interacted_timestamp = millis();

  /*
  Serial.print("RGB_INCREMENT ");
  Serial.println(RGB_INCREMENT);
  Serial.print("DELAY_FOR_COLOR_CHANGE ");
  Serial.println(DELAY_FOR_COLOR_CHANGE);
  */
}

void updateToNextColorInRainbow() {
  int next_color = (current_color + 1) % 3;

  rgbColour[current_color] -= RGB_INCREMENT;
  rgbColour[next_color] += RGB_INCREMENT;
    
  if (rgbColour[current_color] < 0) {
    rgbColour[current_color] = 0;
    rgbColour[next_color] = BRIGHTNESS;
   // Serial.println("Next color");
    //Serial.println(current_color);
    //Serial.println(next_color);
    current_color = next_color;
  }
  
}

void  loop()
{
  unsigned long now = millis();
  if ((now - last_interacted_timestamp) > IDLE_TIME_BEFORE_SLEEP) {
    Serial.print("DEEP SLEEP ");
    /*
    Serial.print(now);
    Serial.print(" ");
    Serial.print(last_interacted_timestamp);
    Serial.print(" ");
    Serial.print(now - last_interacted_timestamp);
    Serial.print(" ");
    Serial.print(kIdleTime);
    Serial.println();
    */
    delay(100);
    DeepSleep();
    // Control will resume here when we are woken back up.
    last_interacted_timestamp = now;
    Serial.println("Woke up!");
    return;
  }

  // Turn light off by default.
  pixels.setPixelColor(PIXEL_INDEX, pixels.Color(0,0,0));
  pixels.show();
  digitalWrite(LED_BUILTIN, LOW);

  bool sensor_triggered = digitalRead(SENSOR_PIN) == LOW;

  if (sensor_triggered) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("On!");

    for (int time_spent = 0; time_spent < LIGHT_ON_TIME; time_spent += DELAY_FOR_COLOR_CHANGE) {
      updateToNextColorInRainbow();
      pixels.setPixelColor(PIXEL_INDEX, pixels.Color(rgbColour[0], rgbColour[1], rgbColour[2]));
      pixels.show();
      delay(DELAY_FOR_COLOR_CHANGE);
    }
    last_interacted_timestamp = now;
  }
}


