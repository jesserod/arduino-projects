/*
  Based originally on http://www.arduino.cc/en/Tutorial/TFTPong
 */
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>
#include <flat_long_map.h>
#include <general_util.h>
#include <task_timer.h>

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define FIRST_BUTTON_PIN 28

#define NUM_PLAYERS 2

struct Pins {
  // Map from player ID -> left/right button pin
  FlatLongMap left_button;
  FlatLongMap right_button;
  
  Pins(int num_players) : left_button(num_players), right_button(num_players) {
    int pin = FIRST_BUTTON_PIN;
    for (int player = 0; player < num_players; ++player) {
      pinMode(pin, INPUT_PULLUP);
      left_button.Insert(player, pin++);
      pinMode(pin, INPUT_PULLUP);
      right_button.Insert(player, pin++);
    }
  }
};

Pins PINS(NUM_PLAYERS);



MCUFRIEND_kbv tft;

int paddleX = 130;
int paddleY = 130;
int oldPaddleX = paddleX;
int oldPaddleY = paddleY;

int ballDirectionX = 1;
int ballDirectionY = 1;

int ballSpeed = 50; // Pixels per second
int paddleSpeed = 30; // Pixels per second
int paddleHeight = 5;
int paddleWidth = 20;
int ballSize = 5;

int ballX, ballY, oldBallX, oldBallY;

int SCREEN_WIDTH = 0;
int SCREEN_HEIGHT = 0;


TaskTimer TICK(0);

long TICK_MILLIS = 20;

int ballPixelsPerTick = ballSpeed * TICK_MILLIS / 1000 /* millis per sec */;
int paddlePixelsPerTick = paddleSpeed * TICK_MILLIS / 1000 /* millis per sec */;

void SetupTFT() {
  tft.reset();
  tft.begin(0x9486);
  SCREEN_WIDTH = tft.width();
  SCREEN_HEIGHT = tft.height();
}

void setup(void) {
  // Serial.begin(9600);
  // if (!Serial) delay(3000);

  SetupTFT();

  TICK.RegisterTask(0);
  TICK.SetTimer(0, TICK_MILLIS);
  
  tft.fillScreen(BLACK);
}


void loop() {





  
  // TODO LEFT OFF: Set up separate timers per entity so each can have its own speed.
  // Basically, each entity should have a set amount of time it must wait before
  // it moves one pixel.




  
  if (TICK.IsTimeUp(0)) {
    TICK.SetTimer(0, TICK_MILLIS);
  
    int newX = paddleX;
    int newY = paddleY;

  
    if (digitalRead(PINS.right_button.Get(0)) == LOW) {
      newX += paddlePixelsPerTick;
    }
    if (digitalRead(PINS.left_button.Get(0)) == LOW) {
      newX -= paddlePixelsPerTick;
    }
    
    paddleX = GeneralUtil::Clamp(newX, 0 - paddleWidth/2, SCREEN_WIDTH - paddleWidth/2);
    paddleY = GeneralUtil::Clamp(newY, 0 - paddleHeight/2, SCREEN_HEIGHT - paddleHeight/2);
  
  
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

    moveBall();
  }
  TICK.Update();
}

// this function determines the ball's position on screen
void moveBall() {
  // if the ball goes offscreen, reverse the direction:
  if (ballX > SCREEN_WIDTH || ballX < 0) {
    ballDirectionX = -ballDirectionX;
  }

  if (ballY > SCREEN_HEIGHT || ballY < 0) {
    ballDirectionY = -ballDirectionY;
  }

  // check if the ball and the paddle occupy the same space on screen
  if (inPaddle(ballX, ballY, paddleX, paddleY, paddleWidth, paddleHeight)) {
    ballDirectionX = -ballDirectionX;
    ballDirectionY = -ballDirectionY;
  }

  // update the ball's position
  ballX += ballDirectionX * ballPixelsPerTick;
  ballY += ballDirectionY * ballPixelsPerTick;

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
