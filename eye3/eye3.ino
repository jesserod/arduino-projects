//#include <avr/sleep.h>
//#include <avr/power.h>
//#include <avr/wdt.h>
#include "LowPower.h"



/* Assumes we have two pull-up momentary buttons on these pins where one
 *  button puts the Arduino to sleep indefinitely and the other one wakes it up.
 */

//int LED_OUTPUT_PIN = 7;
//int LED_OUTPUT_PIN2 = 6;

int CAMERA_POWER_PIN = 7; // 
int CAMERA_POWER_ON = HIGH; // 
int CAMERA_POWER_OFF = LOW; // 
int CAMERA_POWERUP_WAIT_TIME = 11500;
int CAMERA_POWERDOWN_WAIT_TIME = 0000;

int CAMERA_TRIGGER_PIN = 5;
int CAMERA_TRIGGER_ON = LOW;
int CAMERA_TRIGGER_OFF = HIGH;
int CAMERA_TRIGGER_VIDEO_TIME = 600;
int CAMERA_TRIGGER_PICTURE_TIME = 50;
int WAIT_AFTER_PIC = 3000;
//int WAIT_AFTER_VIDEO = 1000;

// TODO: Use real-time-clock to avoid oversleeping!
// TODO: Use real-time-clock to avoid oversleeping!
// TODO: Use real-time-clock to avoid oversleeping!
// TODO: Use real-time-clock to avoid oversleeping!
#define SLEEP_TIME 12
int sleep_time_remaining = SLEEP_TIME;

void takePicture() {
  digitalWrite(CAMERA_TRIGGER_PIN, CAMERA_TRIGGER_ON);
  delay(CAMERA_TRIGGER_PICTURE_TIME);
  digitalWrite(CAMERA_TRIGGER_PIN, CAMERA_TRIGGER_OFF);
  delay(WAIT_AFTER_PIC);
}

void takeVideo(int videoTime) {
  digitalWrite(CAMERA_TRIGGER_PIN, CAMERA_TRIGGER_ON);
  delay(CAMERA_TRIGGER_VIDEO_TIME);
  digitalWrite(CAMERA_TRIGGER_PIN, CAMERA_TRIGGER_OFF);
  delay(videoTime);
  takePicture(); // To stop recording, includes a delay
}

void setup() {
  // Serial.begin(9600);
  
    pinMode(CAMERA_TRIGGER_PIN, OUTPUT);
    pinMode(CAMERA_POWER_PIN, OUTPUT);

    digitalWrite(CAMERA_TRIGGER_PIN, CAMERA_TRIGGER_OFF);
    digitalWrite(CAMERA_POWER_PIN, CAMERA_POWER_OFF);
}

void loop() {
  if (sleep_time_remaining <= 0) {
    // Serial.println("POWER UP"); delay(100);
    digitalWrite(CAMERA_POWER_PIN, CAMERA_POWER_ON);
    delay(CAMERA_POWERUP_WAIT_TIME);

    //Serial.println("VID"); delay(100);
    takeVideo(8000);
    //Serial.println("PIC"); delay(100);
    takePicture();    
    
    // TODO: Undo this, we will want to turn off camera
    digitalWrite(CAMERA_POWER_PIN, CAMERA_POWER_OFF);
    sleep_time_remaining = SLEEP_TIME;
  } else {
    sleep_time_remaining -= 2;  // 4 seconds per sleep, keep in sync wth LowPower.powerDown call  
  }
  // Enter power down state for 4 s with ADC and BOD module disabled
  // Serial.println("Sleeping"); delay(100);
  LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF); 
}
