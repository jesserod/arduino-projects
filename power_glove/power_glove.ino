#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif


#define LED_PIN LED_BUILTIN
#define SENSOR_PIN 3
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


void setup()
{
  pinMode(LED_PIN, OUTPUT); //define LED as a output port
  pinMode(SENSOR_PIN, INPUT_PULLUP); //define switch as a output port
  pixels.begin(); // This initializes the NeoPixel library
  Serial.begin(9600);
  Serial.print("RGB_INCREMENT ");
  Serial.println(RGB_INCREMENT);
  Serial.print("DELAY_FOR_COLOR_CHANGE ");
  Serial.println(DELAY_FOR_COLOR_CHANGE);
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
  pixels.setPixelColor(PIXEL_INDEX, pixels.Color(0,0,0));
  pixels.show();
  digitalWrite(LED_PIN, LOW);
    
  int sensor_reading = digitalRead(SENSOR_PIN); //read the value of the digital interface 3 assigned to val
  if (sensor_reading == LOW) //when the switch sensor have signal, LED blink
  {
    digitalWrite(LED_PIN, HIGH);
    
    Serial.println("On!");

    for (int time_spent = 0; time_spent < LIGHT_ON_TIME; time_spent += DELAY_FOR_COLOR_CHANGE) {
      updateToNextColorInRainbow();
      pixels.setPixelColor(PIXEL_INDEX, pixels.Color(rgbColour[0], rgbColour[1], rgbColour[2]));
      pixels.show();
      delay(DELAY_FOR_COLOR_CHANGE);
    }

    
  }

}


