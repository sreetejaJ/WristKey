#ifndef Ball
#define Ball

class Ball {
    int posX;
    int posY;

    int velX;
    int velY;

    int ballSize = 5;

    void begin(int x, int y) {
      posX = x;
      posY = y;
      velY = int(random(3, 5));
      if (int(random(0, 2)) == 0) {
        velY = -velY;
      }
      if (int(random(0, 2)) == 0) {
        velX = -velX;
      }
    }

    void getUpdate(int &x, int &y, int &sizeB) {
      //u8g2.drawBox(posX - ballSize / 2, posY - ballSize / 2, ballSize, ballSize);
      x = posX;
      y = posY;
      sizeB = ballSize;
    }

    void move() {
      posX += velX;
      posY += velY;
      if ((posY <= ballSize) || (posY >= SCREEN_HEIGHT - ballSize)) {
        velY = -velY;
      }
    }

    void checkBounce(int batX, int batY, int batLen, int compX, int compY, int batWidth) {
      if ((posX <= batX + batWidth / 2) && (posX >= batX - batWidth / 2)) {
        if ((posY <= batY + batLen / 2) && (posY >= batY - batLen / 2)) {
          velX = -velX;
        }
      }

      if ((posX <= compX + batWidth / 2) && (posX >= compX - batWidth / 2)) {
        if ((posY <= compY + batLen / 2) && (posY >= compY - batLen / 2)) {
          velX = -velX;
        }
      }
    }
};

#endif

