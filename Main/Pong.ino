void Pong() {
  int X = SCREEN_WIDTH / 2;
  int Y = SCREEN_HEIGHT / 2;

  int playerY = Y;
  int compY = Y;
  int playerX = 3;

  int compX = SCREEN_WIDTH - PAD_WIDTH - 3;
  int velX = 6;
  int velY = random(2, 6);

  unsigned long currentTime = millis();
  unsigned int frameRate = 50;
  int ballSize = 5;

  if (int(random(0, 2)) == 0) {
    velY = -velY;
  }
  if (int(random(0, 2)) == 0) {
    velX = -velX;
  }
  int lastPlayerY = playerY;
  int val;
  while (true) {
    u8g2.clearBuffer();
    setField();
    score();
    move(X, Y, velX, velY, ballSize);
    updateBall(X, Y, ballSize);
    if (X < 64) {
      movePad(playerY, PAD_HEIGHT, Y, SPEED);
    } else {
      movePad(compY, PAD_HEIGHT, Y, SPEED + 3);
    }
    updateBat(playerX, playerY, PAD_WIDTH, PAD_HEIGHT);
    updateBat(compX, compY, PAD_WIDTH, PAD_HEIGHT);
    checkBounce(playerX, playerY, PAD_HEIGHT, compX, compY, PAD_WIDTH, X, Y, velX, ballSize / 2);
    if (checkLoss(X, playerX, compX + PAD_WIDTH)) {
      Serial.println("Game Over");

      X = SCREEN_WIDTH / 2;
      Y = SCREEN_HEIGHT / 2;

      velY = random(2, 6);

      if (int(random(0, 2)) == 0) {
        velY = -velY;
      }
      if (int(random(0, 2)) == 0) {
        velX = -velX;
      }
    }
    u8g2.sendBuffer();
  }
}

void setField() {
  u8g2.drawBox(63, 0, 1, 5);
  u8g2.drawBox(63, 10, 1, 5);
  u8g2.drawBox(63, 20, 1, 5);
  u8g2.drawBox(63, 30, 1, 5);
  u8g2.drawBox(63, 40, 1, 5);
  u8g2.drawBox(63, 50, 1, 5);
  u8g2.drawBox(63, 60, 1, 5);
}

void score() {
  u8g2.setFont(u8g2_font_t0_17_tf);
  u8g2.setCursor(40, 15);
  u8g2.print("15");
  u8g2.setCursor(70, 15);
  u8g2.print("03");
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
  if ((posX < batX + batWidth) && (posX > batX - batWidth)) {
    if ((posY <= batY + batLen) && (posY >= batY)) {
      velX = -velX;
    }
  }

  if ((posX < compX + batWidth ) && (posX > compX - batWidth + ballSize)) {
    if ((posY <= compY + batLen ) && (posY >= compY)) {
      velX = -velX;
    }
  }
}

void updateBat(int posX, int posY, int batWidth, int batHeight) {
  u8g2.drawBox(posX, posY, batWidth, batHeight);
}

void movePad(int &posY, int batHeight, int ballY, int Speed) {
  if (posY + batHeight / 2 > ballY) {
    posY -= Speed;
  }
  if (posY + batHeight / 2 < ballY) {
    posY += Speed;
  }
  if (posY <= 0) {
    posY = 0;
  }
  if (posY >= SCREEN_HEIGHT - batHeight) {
    posY = SCREEN_HEIGHT - batHeight;
  }
}

bool checkLoss(int ballX, int playerLim, int compLim) {
  if (ballX <= playerLim || ballX >= compLim) {
    return true;
  }
  return false;
}

