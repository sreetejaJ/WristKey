const byte encA = 36;
const byte encB = 37;
const byte button = 38;
int pos = 0;
int numberOfInterrupts = 0;


void setup() {

  Serial.begin(115200);
  pinMode(encA, INPUT);
  pinMode(encB, INPUT);
  pinMode(button, INPUT);
}

void loop() {
  int val = getInput();
  if(val != 10){
    pos += val;
  }else{
    Serial.println("Button Pressed");
  }
  Serial.println(pos);
}

int getInput() {
  if (!digitalRead(button)) {
    while (!digitalRead(button)) {

    }
    Serial.println("Button Pressed");
    return 10;
  }
  if (!digitalRead(encA) && digitalRead(encB)) {
    while (!digitalRead(encA) || !digitalRead(encB)) {
      delay(10);
    }
    return -1;
  } else if (!digitalRead(encB) && digitalRead(encA)) {
    while (!digitalRead(encA) || !digitalRead(encB)) {
      delay(10);
    }
    return 1;
  }
  return 0;
}

