/*

 TFT Pong

 This example for the Arduino screen reads the values
 of 2 potentiometers to move a rectangular platform
 on the x and y axes. The platform can intersect
 with a ball causing it to bounce.

 This example code is in the public domain.

 Created by Tom Igoe December 2012
 Modified 15 April 2013 by Scott Fitzgerald

 http://www.arduino.cc/en/Tutorial/TFTPong

 */

// #include <TFT.h>  // Arduino LCD library
// #include <SPI.h>
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define leftPin 28
#define rightPin 29
#define upPin 30
#define downPin 31


void setup(void);
void loop(void);


uint16_t g_identifier;




// variables for the position of the ball and paddle
int paddleX = 130;
int paddleY = 130;
int oldPaddleX = paddleX;
int oldPaddleY = paddleY;

int ballDirectionX = 1;
int ballDirectionY = 1;

int ballSpeed = 10; // lower numbers are faster
int paddleSpeed = 50; // lower numbers are faster
int paddleHeight = 5;
int paddleWidth = 20;
int ballSize = 5;

int ballX, ballY, oldBallX, oldBallY;

int clamp(int val, int low, int high) {
  if (val > high) {
    val = high;
  }
  if (val < low) {
    val = low;
  }
  return val;
}

void setup(void) {
    Serial.begin(9600);
    uint32_t when = millis();
    //    while (!Serial) ;   //hangs a Leonardo until you connect a Serial
    if (!Serial) delay(5000);           //allow some time for Leonardo
    Serial.println("Serial took " + String((millis() - when)) + "ms to start");

    static uint16_t identifier;
    tft.reset();                 //we can't read ID on 9341 until begin()
    g_identifier = tft.readID(); //

    Serial.print("ID = 0x");
    Serial.println(g_identifier, HEX);

    if (g_identifier == 0x00D3) g_identifier = 0x9481; // write-only shield
    if (g_identifier == 0xFFFF) g_identifier = 0x9341; // serial
        g_identifier = 0x9486;                             // force ID
    tft.begin(g_identifier);

    // Start of non-boilerplate code
    
    pinMode(leftPin, INPUT_PULLUP);
    pinMode(rightPin, INPUT_PULLUP);
    pinMode(upPin, INPUT_PULLUP);
    pinMode(downPin, INPUT_PULLUP);
    
    tft.fillScreen(BLACK);
}


void loop() {

  // save the width and height of the screen
  int myWidth = tft.width();
  int myHeight = tft.height();

  // map the paddle's location to the position of the potentiometers
  //paddleX = map(analogRead(paddleXPin), 512, -512, 0, myWidth) - 20 / 2;
  //paddleY = map(analogRead(paddleYPin), 512, -512, 0, myHeight) - 5 / 2;
  if (millis() % paddleSpeed < 2) {
    int newX = paddleX;
    int newY = paddleY;

    Serial.println(String(digitalRead(upPin)) + " " + String(digitalRead(downPin)) + " " + String(digitalRead(leftPin)) + " " + String(digitalRead(rightPin)));
    
    if (digitalRead(upPin) == LOW) {
      newY++;
    }
    if (digitalRead(downPin) == LOW) {
      newY--;
    }
    if (digitalRead(rightPin) == LOW) {
      newX++;
    }
    if (digitalRead(leftPin) == LOW) {
      newX--;
    }
    
    paddleX = clamp(newX, 0 - paddleWidth/2, myWidth - paddleWidth/2);
    paddleY = clamp(newY, 0 - paddleHeight/2, myHeight - paddleHeight/2);
  }
  
  // set the fill color to black and erase the previous
  // position of the paddle if different from present
  if (oldPaddleX != paddleX || oldPaddleY != paddleY) {
    tft.fillRect(oldPaddleX, oldPaddleY, paddleWidth, paddleHeight, BLACK);
  }

  // draw the paddle on screen, save the current position
  // as the previous.

  tft.fillRect(paddleX, paddleY, paddleWidth, paddleHeight, WHITE);
  oldPaddleX = paddleX;
  oldPaddleY = paddleY;

  // update the ball's position and draw it on screen
  if (millis() % ballSpeed < 2) {
    moveBall();
  }
}

// this function determines the ball's position on screen
void moveBall() {
  // if the ball goes offscreen, reverse the direction:
  if (ballX > tft.width() || ballX < 0) {
    ballDirectionX = -ballDirectionX;
  }

  if (ballY > tft.height() || ballY < 0) {
    ballDirectionY = -ballDirectionY;
  }

  // check if the ball and the paddle occupy the same space on screen
  if (inPaddle(ballX, ballY, paddleX, paddleY, paddleWidth, paddleHeight)) {
    ballDirectionX = -ballDirectionX;
    ballDirectionY = -ballDirectionY;
  }

  // update the ball's position
  ballX += ballDirectionX;
  ballY += ballDirectionY;

  // erase the ball's previous position
  if (oldBallX != ballX || oldBallY != ballY) {
    tft.fillRect(oldBallX, oldBallY, ballSize, ballSize, BLACK);
  }


  // draw the ball's current position
  tft.fillRect(ballX, ballY, ballSize, ballSize, WHITE);

  oldBallX = ballX;
  oldBallY = ballY;

}

// this function checks the position of the ball
// to see if it intersects with the paddle
boolean inPaddle(int x, int y, int rectX, int rectY, int rectWidth, int rectHeight) {
  boolean result = false;

  if ((x >= rectX && x <= (rectX + rectWidth)) &&
      (y >= rectY && y <= (rectY + rectHeight))) {
    result = true;
  }

  return result;
}
