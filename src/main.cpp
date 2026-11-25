#include <Arduino.h>

// Line sensors, left to right
const int pins[8]       = {A7, A6, A5, A4, A3, A2, A1, A0};
const int weights[8]    = {-7, -5, -3, -1, 1, 3, 5, 7};
const int thresholds[8] = {645, 694, 654, 632, 587, 606, 535, 510};

const int TRIG_PIN = 12, ECHO_PIN = 13;

// Obstacle avoidance steps: right speed, left speed, duration (ms)
// Duration 0 = keep going until the line is found again
struct Step { int right, left, ms; };
const Step steps[] = {
  {0, 0, 150},       // stop
  {-150, 150, 400},  // turn right
  {170, 170, 700},   // forward
  {150, -150, 400},  // turn left
  {170, 170, 900},   // pass obstacle
  {150, -150, 400},  // turn towards line
  {130, 130, 0},     // search for line
  {-150, 150, 400}   // align with line
};
const int STEP_COUNT = sizeof(steps) / sizeof(steps[0]);

int step = -1;  // -1 = normal line following
float filtered = 0;
int lastDir = 0, obstacleCount = 0;
unsigned long stepTimer = 0, lastPing = 0, sharpUntil = 0, lostSince = 0, blackSince = 0;

// Right: pins 3 (fwd) / 5 (back). Left: pins 6 (fwd) / 9 (back)
void drive(int right, int left) {
  analogWrite(3, constrain(right, 0, 255));
  analogWrite(5, constrain(-right, 0, 255));
  analogWrite(6, constrain(left, 0, 255));
  analogWrite(9, constrain(-left, 0, 255));
}

// Distance in cm (999 = nothing detected)
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  return duration ? duration * 0.0343 / 2 : 999;
}

// Sum of weights of sensors on the line, and how many see it
void readLine(int &total, int &active) {
  total = active = 0;
  for (int i = 0; i < 8; i++) {
    if (analogRead(pins[i]) > thresholds[i]) {
      total += weights[i];
      active++;
    }
  }
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  drive(0, 0);
}

void loop() {
  unsigned long now = millis();
  int total, active;
  readLine(total, active);

  // --- Obstacle avoidance in progress ---
  if (step >= 0) {
    drive(steps[step].right, steps[step].left);
    bool done = steps[step].ms ? now - stepTimer >= steps[step].ms : active > 0;
    if (done) {
      stepTimer = now;
      if (++step == STEP_COUNT) {  // finished, back to line following
        step = -1;
        filtered = lastDir = 0;
        sharpUntil = lostSince = blackSince = 0;
        drive(0, 0);
      }
    }
    return;
  }

  // --- Obstacle detection (two readings in a row) ---
  if (now - lastPing >= 60) {
    lastPing = now;
    obstacleCount = (getDistance() <= 20) ? obstacleCount + 1 : 0;
    if (obstacleCount >= 2) {
      obstacleCount = 0;
      step = 0;
      stepTimer = now;
      return;
    }
  }

  // --- All black for 200 ms: stop, reverse, and finish ---
  if (active == 8) {
    if (blackSince == 0) blackSince = now;
    if (now - blackSince >= 200) {
      drive(0, 0);
      delay(200);
      drive(-170, -225);
      delay(1000);
      drive(0, 0);
      while (true);
    }
  } else {
    blackSince = 0;
  }

  // --- Lost line ---
  if (active == 0) {
    if (lostSince == 0) lostSince = now;
    if (now - lostSince < 200) return;
    if (lastDir < 0)      drive(240, 70);
    else if (lastDir > 0) drive(70, 240);
    else                  drive(170, 225);
    return;
  }
  lostSince = 0;

  // --- Line following ---
  float error = (float)total / active - 2.0;
  if (abs(error) >= 2.0) sharpUntil = now + 700;
  bool sharp = now < sharpUntil;

  filtered += (sharp ? 0.85 : 0.25) * (error - filtered);

  float correction = (sharp ? 65 : 20) * filtered;
  if (filtered > 2.0) correction *= 1.25;

  if (filtered < -0.5)     lastDir = -1;
  else if (filtered > 0.5) lastDir = 1;

  int boost = (abs(filtered) < 1.0 && !sharp) ? 15 : 0;

  drive(constrain(170 + boost - correction, 90, 255),
        constrain(225 + boost + correction, 90, 255));
}