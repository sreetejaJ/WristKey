

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  long randomNum1;
  long randomNum2;
  randomSeed(analogRead(A0));
  randomNum1 = random(20);
  Serial.println(randomNum1);
  delay(100);
}
