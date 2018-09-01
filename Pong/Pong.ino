#include <Arduino.h>
#include <U8g2lib.h>


U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

int page = 0;
unsigned long timenow = 0;
int updateRate = 100;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define BALL_SIZE 6
#define PAD_HEIGHT 20
#define PAD_WIDTH 5

void setup() {
  // put your setup code here, to run once:
  u8g2.begin();
  u8g2.clearBuffer();
  Serial.begin(115200);
}

void loop() {
  Pong();
}

void Pong() {
  int X = SCREEN_WIDTH / 2;
  int Y = SCREEN_HEIGHT / 2;

  int playerY = Y;
  int compY = Y;
  int playerX = 3;
  int compX = SCREEN_WIDTH - PAD_WIDTH - 3;
  int velX = 3;
  int velY = 0;//random(2, 8);

  unsigned long currentTime = millis();
  unsigned int frameRate = 20;
  int ballSize = 5;

  velY = 0;//int(random(3, 5));
  if (int(random(0, 2)) == 0) {
    velY = -velY;
  }
  if (int(random(0, 2)) == 0) {
    velX = -velX;
  }
  
  while (true) {
    u8g2.clearBuffer();
    updateBall(X, Y, ballSize);
    updateBat(playerX, playerY, PAD_WIDTH, PAD_HEIGHT);
    updateBat(compX, compY, PAD_WIDTH, PAD_HEIGHT);
    checkBounce(playerX, playerY, PAD_HEIGHT, compX, compY, PAD_WIDTH, X, Y, velX, ballSize / 2);
    move(X, Y, velX, velY, ballSize);
    int val = getInput();
    if(val == 10){
      return;
    }
    playerX += val * 5;
    u8g2.sendBuffer();
  }
}


void updateBall(int posX, int posY, int ballSize) {
  u8g2.drawBox(posX - ballSize / 2, posY - ballSize / 2, ballSize, ballSize);
}

void move(int &posX, int &posY, int velX, int &velY, int ballSize) {
  posX += velX;
  posY += velY;
  if ((posY <= ballSize) || (posY >= SCREEN_HEIGHT - ballSize)) {
    velY = -velY;
  }
}

void checkBounce(int batX, int batY, int batLen, int compX, int compY, int batWidth, int posX, int posY, int &velX, int ballSize) {
  if ((posX <= batX + batWidth + ballSize) && (posX > batX - batWidth)) {
    if ((posY <= batY + batLen) && (posY >= batY - batLen )) {
      velX = -velX;
    }
  }

  if ((posX <= compX + batWidth ) && (posX > compX - batWidth + ballSize)) {
    if ((posY <= compY + batLen ) && (posY >= compY - batLen )) {
      velX = -velX;
    }
  }
}

void updateBat(int posX, int posY, int batWidth, int batHeight){   
    u8g2.drawBox(posX, posY - batHeight / 2, batWidth, batHeight);
}
