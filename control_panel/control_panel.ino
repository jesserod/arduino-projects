#include <task_timer.h>
#include <input_tracker.h>
#include <flat_long_map.h>

const int SWITCH_ON = LOW;
const int SWITCH_OFF = HIGH;
const int MAX_FUEL = 100;
const int MAX_BURN_MILLIS = 10*1000;
const double FUEL_PER_MILLI = double(MAX_FUEL) / double(MAX_BURN_MILLIS);

int fuel_level = MAX_FUEL;

// Input pins
#define IGNITION_SWITCH A3
#define COMM_BUTTON 10
#define LAUNCH_BUTTON 9
#define FUEL_SWITCH A1

// Output pins
#define COMM_LIGHT 12
#define LAUNCH_LIGHT 2
#define FUEL1 3
#define FUEL2 4
#define FUEL3 5
#define FUEL4 6
#define FUEL5 7

InputTracker input_tracker(A5);
TaskTimer blink_timer(13);

void setup() {
  Serial.begin(9600);
  bool pullup = true;
  input_tracker.RegisterInputPin(COMM_BUTTON, pullup);
  input_tracker.RegisterInputPin(LAUNCH_BUTTON, pullup);
  input_tracker.RegisterInputPin(FUEL_SWITCH, pullup);
  input_tracker.RegisterInputPin(IGNITION_SWITCH, pullup);
  blink_timer.RegisterTask(LAUNCH_LIGHT);
  pinMode(COMM_LIGHT, OUTPUT);
  pinMode(LAUNCH_LIGHT, OUTPUT);
  pinMode(FUEL1, OUTPUT);
  pinMode(FUEL2, OUTPUT);
  pinMode(FUEL3, OUTPUT);
  pinMode(FUEL4, OUTPUT);
  pinMode(FUEL5, OUTPUT);
  fuel_level = MAX_FUEL;
}

void SetFuelLight(int fuel_pin, int min_level) {
  if (fuel_level >= min_level) {
    digitalWrite(fuel_pin, HIGH);
  } else {
    digitalWrite(fuel_pin, LOW);
  }
}

void loop() {
  SetFuelLight(FUEL1, 1);
  SetFuelLight(FUEL2, 21);
  SetFuelLight(FUEL3, 41);
  SetFuelLight(FUEL4, 61);
  SetFuelLight(FUEL5, 81);

  static unsigned long burn_started_at = 0;
  static int burn_starting_fuel = fuel_level;
  static bool launch_light_on = false;

  if (digitalRead(LAUNCH_BUTTON) == SWITCH_ON) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  if (input_tracker.IsChangedTo(LAUNCH_BUTTON, SWITCH_ON) && fuel_level > 0 && burn_started_at == 0) {
    launch_light_on = true;
    blink_timer.SetTimer(LAUNCH_LIGHT, 500);
    digitalWrite(LAUNCH_LIGHT, HIGH);
    burn_started_at = millis();
    burn_starting_fuel = fuel_level;
  }
  if (input_tracker.IsChangedTo(LAUNCH_BUTTON, SWITCH_ON)) {
    Serial.println("LAUNCH PRESSED");
    Serial.print("fuel: ");
    Serial.println(fuel_level);
    Serial.print("burn at: ");
    Serial.println(burn_started_at);
  }
  if (input_tracker.IsChangedTo(FUEL_SWITCH, SWITCH_ON)) {
    Serial.println("FUEL PRESSED");
    fuel_level = MAX_FUEL;
    burn_starting_fuel = fuel_level;
    // if we are currently burning we have just reset fuel so we have to pretend to restart the burn.
    if (burn_started_at > 0) {
      burn_started_at = millis();
    }
  }
  if (input_tracker.IsChanged(IGNITION_SWITCH)) {
        Serial.print("IGNITION changed, now");
        Serial.println(digitalRead(IGNITION_SWITCH));

  }
  if (input_tracker.IsChangedTo(IGNITION_SWITCH, SWITCH_ON)) {
    Serial.println("IGNITION PRESSED");
  }
   if (input_tracker.IsChangedTo(COMM_BUTTON, SWITCH_ON)) {
    Serial.println("COMM PRESSED");
  }
  if (digitalRead(IGNITION_SWITCH) == SWITCH_ON && fuel_level > 0) {
    bool is_burning = burn_started_at >= 1;
    if (is_burning) {
      unsigned long burn_time = millis() - burn_started_at;
      double fuel_per_millis = double(MAX_FUEL) / double(MAX_BURN_MILLIS);
      fuel_level = burn_starting_fuel - burn_time * FUEL_PER_MILLI;
      if (blink_timer.IsTimeUp(LAUNCH_LIGHT)) {
        if (launch_light_on) {
          digitalWrite(LAUNCH_LIGHT, LOW);
        } else {
          digitalWrite(LAUNCH_LIGHT, HIGH);
        }
        blink_timer.SetTimer(LAUNCH_LIGHT, 500);
        launch_light_on  = !launch_light_on;
      }
    } else {
      // Turn light on because we are ready to launch
      digitalWrite(LAUNCH_LIGHT, HIGH);
    }
  } else {
    // Stop burn.
    digitalWrite(LAUNCH_LIGHT, LOW);
    burn_started_at = 0;
  }

  if (digitalRead(COMM_BUTTON) == SWITCH_ON) {
    digitalWrite(COMM_LIGHT, HIGH);
  } else {
    digitalWrite(COMM_LIGHT, LOW);
  }

  blink_timer.Update();
  input_tracker.Update();
  delay(20);
}
