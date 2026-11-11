
#include <Arduino.h>

// Forward pins
const int leftFwd = 5;    // B1 fwd
const int rightFwd = 6;   // A2 fwd (swapped from 3 to 6)

// Reverse / Rotation pins
const int leftBack = 9;   // B2 back
const int rightBack = 3;  // A1 back (swapped from 6 to 3)

// MH Sensors (R1 = Right, R2 = Left)
const int R1 = 10;
const int R2 = 11;

// Variables to track sensor states and count encoder pulses
int lastRightSensor = HIGH;
int lastLeftSensor = HIGH;
long rightPulses = 0;
long leftPulses = 0;

// Base forward speeds and dynamic right speed adjustment
const int baseLeftSpeed = 200;
const int baseRightSpeed = 200;
int adjustedRightSpeed = 200;

// PI controller
float Kp = -300;
float Ki = -9;

float integral = 0;
float filteredError = 0;

// Adaptive calibration
float learnedCorrection = 0;
const float learningRate = 0.02;
const float maxLearnedCorrection = 50.0;

// Movement timing
unsigned long startTime = 0;
unsigned long lastUpdate = 0;
unsigned long stopTime = 0;

const unsigned long updateInterval = 100;
const unsigned long pauseDuration = 1000;

bool moving = false;
bool waiting = false;

int currentSpeed = 0;
unsigned long currentDuration = 0;

// Function declarations
void stopMotors();
void forward(unsigned long duration, int speed);
void updateMotors();
void setMotorSpeeds(float correction);

void setup() {
  pinMode(leftFwd, OUTPUT);
  pinMode(rightFwd, OUTPUT);
  pinMode(leftBack, OUTPUT);
  pinMode(rightBack, OUTPUT);

  pinMode(R1, INPUT);
  pinMode(R2, INPUT);

  Serial.begin(115200);
  stopMotors();
}

void stopMotors() {
  analogWrite(leftFwd, 0);
  analogWrite(leftBack, 0);
  analogWrite(rightFwd, 0);
  analogWrite(rightBack, 0);
}

void setMotorSpeeds(float correction) {
  int leftSpeed = constrain(
    currentSpeed - correction, 0, 255
  );

  adjustedRightSpeed = constrain(
    currentSpeed + correction, 0, 255
  );

  analogWrite(leftFwd, leftSpeed);
  analogWrite(leftBack, 0);
  analogWrite(rightFwd, 0);
  analogWrite(rightBack, adjustedRightSpeed);
}

void forward(unsigned long duration, int speed) {
  if (moving) return;

  speed = constrain(speed, 0, 255);

  if (speed == 0 || duration == 0) {
    stopMotors();
    return;
  }

  currentSpeed = speed;
  currentDuration = duration;

  // Reset PI but preserve learned calibration
  integral = 0;
  filteredError = 0;
  rightPulses = 0;
  leftPulses = 0;

  lastRightSensor = digitalRead(R1);
  lastLeftSensor = digitalRead(R2);

  startTime = millis();
  lastUpdate = startTime;
  moving = true;

  // Start using the previous calibration
  setMotorSpeeds(learnedCorrection);
}

void updateMotors() {
  if (!moving) return;

  unsigned long now = millis();

  if (now - startTime >= currentDuration) {
    stopMotors();
    moving = false;
    stopTime = now;
    return;
  }

  int rightSensor = digitalRead(R1);
  int leftSensor = digitalRead(R2);

  // Count pulses on state changes (detect transitions from HIGH to LOW)
  if (rightSensor == LOW && lastRightSensor == HIGH) {
    rightPulses++;
  }

  if (leftSensor == LOW && lastLeftSensor == HIGH) {
    leftPulses++;
  }

  lastRightSensor = rightSensor;
  lastLeftSensor = leftSensor;

  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    // Calculate wheel speed difference
    float error = leftPulses - rightPulses;

    // Smooth the error
    filteredError = 0.7 * filteredError + 0.3 * error;

    // PI correction
    integral += filteredError;
    integral = constrain(integral, -100.0, 100.0);

    float correction = Kp * filteredError + Ki * integral;
    correction = constrain(correction, -80.0, 80.0);

    // Combine learned and dynamic correction
    float totalCorrection = constrain(
      learnedCorrection + correction, -80.0, 80.0
    );

    // Clear: Drive Forward with active encoder speed correction
    setMotorSpeeds(totalCorrection);

    // Learn only when measurements are meaningful
    if (leftPulses + rightPulses >= 4 &&
        abs(totalCorrection) < 75.0) {

      // Learn in the same direction as the PI correction
      float learningAdjustment =
        learningRate * Kp * filteredError;

      learnedCorrection += learningAdjustment;
      learnedCorrection = constrain(
        learnedCorrection,
        -maxLearnedCorrection,
        maxLearnedCorrection
      );
    }

    // Diagnostic output
    Serial.print("L pulses: ");
    Serial.print(leftPulses);
    Serial.print(" | R pulses: ");
    Serial.print(rightPulses);

    Serial.print(" | Error: ");
    Serial.print(filteredError);

    Serial.print(" | PI: ");
    Serial.print(correction);

    Serial.print(" | Learned: ");
    Serial.println(learnedCorrection);

    rightPulses = 0;
    leftPulses = 0;
  }
}

void loop() {
  updateMotors();

  if (!moving && !waiting) {
    forward(3000, 200);
    waiting = true;
  }

  if (waiting && !moving &&
      millis() - stopTime >= pauseDuration) {
    waiting = false;
  }
}

// A right motor, A2 fwd, A1 back
// B left motor, B1 fwd, B2 back
// difference fwd (L200-R170 OR L255-R235)
// difference back L255-R-240 OR L_
