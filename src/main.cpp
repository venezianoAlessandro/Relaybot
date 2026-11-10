#include <Arduino.h>

// Forward pins
const int leftFwd = 5;    // B1 fwd
const int rightFwd = 3;   // A2 fwd 

// Reverse
const int leftBack = 9;   // B2 back
const int rightBack = 6;  // A1 back 

// MH Sensors (R1 = Right, R2 = Left)
const int R1 = 10; 
const int R2 = 11; 

// Variables to track sensor states and count encoder pulses
int lastRightSensor = HIGH;
int lastLeftSensor = HIGH;
long rightPulses = 0;
long leftPulses = 0;

// Base forward speeds and dynamic right speed adjustment
const int baseLeftSpeed = 255;
const int baseRightSpeed = 200;
int adjustedRightSpeed = 200;

// PID parameters
float Kp = 8.0;
float Ki = 0.4;
float Kd = 1.5;

float error = 0;
float previousError = 0;
float integral = 0;
float filteredError = 0;
float correction = 0;

// Motor speed control
int leftSpeed = 0;
int rightSpeed = 0;
int targetLeftSpeed = baseLeftSpeed;
int targetRightSpeed = baseRightSpeed;

// Timing
unsigned long lastPIDTime = 0;
unsigned long lastMotorTime = 0;
unsigned long lastPrintTime = 0;

const unsigned long PID_INTERVAL = 150;
const unsigned long MOTOR_INTERVAL = 20;

// Encoder tracking
long previousLeftPulses = 0;
long previousRightPulses = 0;

void setup() {
  pinMode(leftFwd, OUTPUT);
  pinMode(rightFwd, OUTPUT);
  pinMode(leftBack, OUTPUT);
  pinMode(rightBack, OUTPUT);
  
  pinMode(R1, INPUT);
  pinMode(R2, INPUT);
  Serial.begin(9600);

  analogWrite(leftBack, 0);
  analogWrite(rightBack, 0);
}

void loop() {
  unsigned long now = millis();

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

  // PID speed correction
  if (now - lastPIDTime >= PID_INTERVAL) {
    lastPIDTime = now;

    long leftDelta = leftPulses - previousLeftPulses;
    long rightDelta = rightPulses - previousRightPulses;

    previousLeftPulses = leftPulses;
    previousRightPulses = rightPulses;

    // Calculate wheel speed difference
    error = leftDelta - rightDelta;

    // Filter sudden encoder fluctuations
    filteredError = 0.6 * filteredError + 0.4 * error;

    // PID calculations
    integral += filteredError;
    integral = constrain(integral, -100.0, 100.0);

    float derivative = filteredError - previousError;

    correction = Kp * filteredError
               + Ki * integral
               + Kd * derivative;

    correction = constrain(correction, -90.0, 90.0);

    previousError = filteredError;

    // Clear: Drive Forward with active encoder speed correction
    targetLeftSpeed = constrain(
      baseLeftSpeed - correction, 0, 255
    );

    adjustedRightSpeed = constrain(
      baseRightSpeed + correction, 0, 255
    );

    targetRightSpeed = adjustedRightSpeed;
  }

  // Smooth motor acceleration and deceleration
  if (now - lastMotorTime >= MOTOR_INTERVAL) {
    lastMotorTime = now;

    const int maxStep = 5;

    if (leftSpeed < targetLeftSpeed) {
      leftSpeed = min(leftSpeed + maxStep, targetLeftSpeed);
    }
    else if (leftSpeed > targetLeftSpeed) {
      leftSpeed = max(leftSpeed - maxStep, targetLeftSpeed);
    }

    if (rightSpeed < targetRightSpeed) {
      rightSpeed = min(rightSpeed + maxStep, targetRightSpeed);
    }
    else if (rightSpeed > targetRightSpeed) {
      rightSpeed = max(rightSpeed - maxStep, targetRightSpeed);
    }

    analogWrite(leftFwd, leftSpeed);
    analogWrite(leftBack, 0);
    analogWrite(rightFwd, rightSpeed);
    analogWrite(rightBack, 0);
  }

  // Monitor PID performance
  if (now - lastPrintTime >= 500) {
    lastPrintTime = now;

    Serial.print("Left: ");
    Serial.print(leftSpeed);

    Serial.print(" Right: ");
    Serial.print(rightSpeed);

    Serial.print(" Error: ");
    Serial.print(filteredError);

    Serial.print(" Correction: ");
    Serial.println(correction);
  }
}

// A right motor, A2 fwd, A1 back 
// B left motor, B1 fwd, B2 back 
// difference fwd (L200-R170 OR L255-R235) 
// difference back L255-R-240 OR L_
