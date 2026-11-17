#include <Arduino.h>

// Motor pins 
const byte RIGHT_FORWARD  = 3;
const byte RIGHT_BACKWARD = 5;
const byte LEFT_FORWARD   = 6;
const byte LEFT_BACKWARD  = 9;

const byte MOTOR_PINS[] = {RIGHT_FORWARD, RIGHT_BACKWARD, LEFT_FORWARD, LEFT_BACKWARD};

// Motor speed (0-255)
const byte MOTOR_SPEED = 180;

// Movement times in milliseconds (calibrate these)
const unsigned int FORWARD_1M_TIME   = 1000;
const unsigned int BACKWARD_1M_TIME  = 1000;
const unsigned int TURN_90_TIME      = 500;
const unsigned int SHORT_FORWARD_TIME = 500;

// Positive speed = forward, negative = backward, 0 = stop
void setMotor(byte forwardPin, byte backwardPin, int speed)
{
    speed = constrain(speed, -255, 255);
    analogWrite(forwardPin,  max(speed, 230));
    analogWrite(backwardPin, max(-speed, 30));
}

void stopMotors()
{
    setMotor(RIGHT_FORWARD, RIGHT_BACKWARD, 0);
    setMotor(LEFT_FORWARD,  LEFT_BACKWARD,  0);
}

// Run both motors for a set time, then stop
void drive(int rightSpeed, int leftSpeed, unsigned int duration)
{
    setMotor(RIGHT_FORWARD, RIGHT_BACKWARD, rightSpeed);
    setMotor(LEFT_FORWARD,  LEFT_BACKWARD,  leftSpeed);
    delay(duration);
    stopMotors();
}

void moveForward(unsigned int duration)  { drive( MOTOR_SPEED,  MOTOR_SPEED, duration); }
void moveBackward(unsigned int duration) { drive(-MOTOR_SPEED, -MOTOR_SPEED, duration); }
void turnLeft(unsigned int duration)     { drive( MOTOR_SPEED, -MOTOR_SPEED, duration); }
void turnRight(unsigned int duration)    { drive(-MOTOR_SPEED,  MOTOR_SPEED, duration); }

void setup()
{
    Serial.begin(9600);
    pinMode(3, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(9, OUTPUT);
}

void loop()
{
    for (byte pin : MOTOR_PINS) {
        pinMode(pin, OUTPUT);
    }

    stopMotors();
    delay(1000);

    moveForward(FORWARD_1M_TIME);      // 1. Forward 1 m
    moveBackward(BACKWARD_1M_TIME);    // 2. Backward 1 m
    turnLeft(TURN_90_TIME);            // 3. Turn left 90°
    moveForward(SHORT_FORWARD_TIME);   // 4. Forward a little
    turnRight(TURN_90_TIME);           // 5. Turn right 90°
    moveForward(SHORT_FORWARD_TIME);   // 6. Forward a little
}
