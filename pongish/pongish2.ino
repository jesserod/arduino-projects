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

// Arcade buttons
#define FIRST_BUTTON_PIN 44

// Smaller buttons in between.
#define SECONDARY_FIRST_BUTTON_PIN 32

#define NUM_PLAYERS 2

void resetBallTimer();
void resetPaddleTimer(int player);

class Paddle {
 public:
  int x, y;
  int leftButtonPin = -1;
  int rightButtonPin = -1;

  void Init(int startX, int startY) {
    x = startX;
    y = startY;
  }
  
  // Paddle(int startX_, int startY_) : x(startX), y(startY), oldX(startX), oldY(startY), id(id_) {}

  int SetupPins(int nextAvailablePin) {
    leftButtonPin = nextAvailablePin;
    pinMode(leftButtonPin, INPUT_PULLUP);
    rightButtonPin = leftButtonPin + 1;
    pinMode(rightButtonPin, INPUT_PULLUP);
    return rightButtonPin + 1;
  }
};

Paddle paddles[NUM_PLAYERS];


MCUFRIEND_kbv tft;

int ballDirectionX = 1;
int ballDirectionY = 1;

int ballSpeed = 100; // Pixels per second
int paddleSpeed = 150; // Pixels per second
int paddleHeight = 5;
int paddleWidth = 40;
int ballSize = 5;

int ballX, ballY, oldBallX, oldBallY;

int SCREEN_WIDTH = 0;
int SCREEN_HEIGHT = 0;

int ballMillisPerPixel = 1000 / ballSpeed;
int paddleMillisPerPixel = 1000 / paddleSpeed;

TaskTimer timer(1 + NUM_PLAYERS);  // Ball + players

#define BALL_TASK 100


class Vector {
 public:
  Vector() { Update(0, 0); }
  Vector(float x, float y) { Update(x, y); }
  Vector(const Vector& other) { Update(other.X(), other.Y()); }
  
  float X() { return x_; }
  float Y() { return y_; }
  
  float SetX(float new_x) { x_ = new_x; }
  float SetY(float new_y) { y_ = new_y; }
  void Update(float x, float y) {   SetX(x); SetY(y); }

  // Returns angle in radians.
  float Angle() { return atan2(y, x); }
  float Length() { return sqrt(x_ * x_ + y_ * y_); }

  void SetLength(float new_length) {
    float theta_radians = Angle();
    float new_x = cos(theta) * new_length;  // cos(theta) = x / r
    float new_y = sin(theta) * new_length;  // sin(theta) = y / r
    Update(new_x, new_y);
  }
  
  void SetAngle(float new_angle_radians) {
    float length = Length();
    float new_x = cos(new_angle_radians) * length;  // cos(theta) = x / r
    float new_y = sin(new_angle_radians) * length;  // sin(theta) = y / r
    Update(new_x, new_y);
  }
  
 protected:
  float x_, y_;
}

class Velocity : public Vector {
 public:
  Velocity() { Update(0, 0); }
  Velocity(float x, float y) { Update(x, y); }
  Velocity(const Velocity& other) { Update(other.X(), other.Y()); }

  float Speed() { return Length(); }

  void SetSpeed(float new_speed) {
    SetLength(new_speed);
  }
}

class MovingThing {
 public:
  MovingThing() { }

 protected:
  Velocity velocity;
  
}

void SetupTFT() {
  tft.reset();
  tft.begin(0x9486);
  SCREEN_WIDTH = tft.width();
  SCREEN_HEIGHT = tft.height();
}

void setup(void) {
  // Serial.begin(9600);
  // if (!Serial) delay(3000);

  // As a safety precaution
  for (int i = 23; i <= 53; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  SetupTFT();

  ballX = oldBallX = SCREEN_WIDTH / 2;
  ballY = oldBallY = SCREEN_HEIGHT / 2;

  timer.RegisterTask(BALL_TASK);
  resetBallTimer();

  const int PADDLE_GAP = 35;  // How far from edge of screen should paddle be.
  int nextPin = FIRST_BUTTON_PIN;
  for (int i = 0; i < NUM_PLAYERS; ++i) {
    int paddleX = SCREEN_WIDTH / 2;
    int paddleY = i == 0 ? PADDLE_GAP : SCREEN_HEIGHT - PADDLE_GAP;
    paddles[i].Init(paddleX, paddleY);
    nextPin = paddles[i].SetupPins(nextPin);
  }

  for (int i = 0; i < NUM_PLAYERS; ++i) {
    timer.RegisterTask(i);
    resetPaddleTimer(i);
  }

  tft.fillScreen(BLACK);
}

void maybeMovePaddle(int player) {
  Paddle& paddle = paddles[player];
  int oldX = paddle.x;
  int leftDirection = player == 0 ? 1 : -1;
  if (digitalRead(paddle.leftButtonPin) == LOW) {
    paddle.x += leftDirection;
  }
  if (digitalRead(paddle.rightButtonPin) == LOW) {
    paddle.x += -1 * leftDirection;
  }
  
  paddle.x = GeneralUtil::Clamp(paddle.x, 0 - paddleWidth/2, SCREEN_WIDTH - paddleWidth/2);

  if (paddle.x != oldX) {
    tft.fillRect(oldX, paddle.y, paddleWidth, paddleHeight, BLACK);
  }
  int color = player == 0 ? WHITE : RED;
  tft.fillRect(paddle.x, paddle.y, paddleWidth, paddleHeight, color);  
}

void resetBallTimer() {
  timer.SetTimer(BALL_TASK, ballMillisPerPixel);
}

void resetPaddleTimer(int player) {
  timer.SetTimer(player, paddleMillisPerPixel);
}


void loop() {
  if (timer.IsTimeUp(BALL_TASK)) {
    moveBall();
    resetBallTimer();
  }

  for (int player = 0; player < NUM_PLAYERS; ++player) {
     if (timer.IsTimeUp(player)) {
       maybeMovePaddle(player);
       resetPaddleTimer(player);
     }
  }
  timer.Update();
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
  for (int i = 0; i < NUM_PLAYERS; ++i) {
    if (inPaddle(ballX, ballY, paddles[i].x, paddles[i].y, paddleWidth, paddleHeight)) {
      ballDirectionX = -ballDirectionX;
      ballDirectionY = -ballDirectionY;
      break;
    }
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
