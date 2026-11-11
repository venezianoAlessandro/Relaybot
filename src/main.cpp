const int Lwheel = 3; // A3 for forward left wheel
  const int Rwheel = 5; // A5 for forward right wheel

  const int LwheelBack = 6; // A6 is for the Left wheel to go back
  const int RwheelBack = 9; //assumption it's the back right wheel?


  const int baseLeftSpeed = 194;
  const int baseRightSpeed = 200;

  const int baseLeftBackSpeed = 233;
  const int baseRightBackSpeed = 190;



void setup() {

pinMode(Lwheel, OUTPUT);
pinMode(Rwheel, OUTPUT);

pinMode(LwheelBack, OUTPUT);
pinMode(RwheelBack, OUTPUT);

}

void loop() {

analogWrite(Lwheel, 0);
analogWrite(Rwheel, 0);

delay(1000);

analogWrite(Lwheel, baseLeftSpeed);
analogWrite(Rwheel, baseRightSpeed);

delay (3000);

analogWrite(Lwheel, 0);
analogWrite(Rwheel, 0);

analogWrite(LwheelBack, baseLeftBackSpeed);
analogWrite(RwheelBack, baseRightBackSpeed);

delay (3000);

analogWrite(LwheelBack, 0);
analogWrite(RwheelBack, 0);

}