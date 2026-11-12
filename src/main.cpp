  #include <Arduino.h>

    const int Lwheelfoward = 3; // A3 for forward left wheel
    const int Rwheelforward = 5; // A5 for forward right wheel

    const int LwheelBack = 6; // A6 is for the Left wheel to go back
    const int RwheelBack = 9; //assumption it's the back right wheel?


    const float baseLeftSpeed = 194;
    const float baseRightSpeed = 200;

    const float baseLeftBackSpeed = 233;
    const float baseRightBackSpeed = 190;

    unsigned long startTime = 0; // Start time in milliseconds
    unsigned long currentTime = 0; // Current time in milliseconds



    void setup() {

    pinMode(Lwheelfoward, OUTPUT);
    pinMode(Rwheelforward, OUTPUT);

    pinMode(LwheelBack, OUTPUT);
    pinMode(RwheelBack, OUTPUT);

  startTime = millis();
  }

  void loop() {
  currentTime = millis();

  analogWrite(Lwheelfoward, baseLeftSpeed);
  analogWrite(Rwheelforward, baseRightSpeed);

  analogWrite(LwheelBack, baseLeftBackSpeed);
  analogWrite(RwheelBack, baseRightBackSpeed);

  }