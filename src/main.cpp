#include <Arduino.h>

// Motor pins
const byte RIGHT_FORWARD = 3;
const byte RIGHT_BACKWARD = 5;
const byte LEFT_FORWARD = 6;
const byte LEFT_BACKWARD = 9;

const byte MOTOR_SPEED = 180; // 0-255

// Calibrate these by testing on your floor
const unsigned int ONE_METER_TIME = 1000; // ms to travel 100 cm
const unsigned int TURN_90_TIME = 500;    // ms to turn 90 degrees

// speed > 0 = forward, speed < 0 = backward, 0 = stop
void setMotor(byte forwardPin, byte backwardPin, int speed)
{
    if (speed > 0)
    {
        analogWrite(forwardPin, speed);
        analogWrite(backwardPin, 0);
    }
    else if (speed < 0)
    {
        analogWrite(forwardPin, 0);
        analogWrite(backwardPin, -speed);
    }
    else
    {
        analogWrite(forwardPin, 0);
        analogWrite(backwardPin, 0);
    }
}

void stopMotors()
{
    setMotor(RIGHT_FORWARD, RIGHT_BACKWARD, 0);
    setMotor(LEFT_FORWARD, LEFT_BACKWARD, 0);
}

// Run both motors for a set time, then stop
void drive(int rightSpeed, int leftSpeed, unsigned int duration)
{
    setMotor(RIGHT_FORWARD, RIGHT_BACKWARD, rightSpeed);
    setMotor(LEFT_FORWARD, LEFT_BACKWARD, leftSpeed);
    delay(duration);
    stopMotors();
    delay(500); // short pause so the robot settles before the next move
}

void moveForward(unsigned int duration) { drive(MOTOR_SPEED, MOTOR_SPEED, duration); }
void moveBackward(unsigned int duration) { drive(-MOTOR_SPEED, -MOTOR_SPEED, duration); }
void turnLeft(unsigned int duration) { drive(MOTOR_SPEED, -MOTOR_SPEED, duration); }
void turnRight(unsigned int duration) { drive(-MOTOR_SPEED, MOTOR_SPEED, duration); }

void setup()
{
    Serial.begin(9600);
    Serial.println("BOOT");   // prints every time the Nano restarts

    pinMode(RIGHT_FORWARD,  OUTPUT);
    pinMode(RIGHT_BACKWARD, OUTPUT);
    pinMode(LEFT_FORWARD,   OUTPUT);
    pinMode(LEFT_BACKWARD,  OUTPUT);

    stopMotors();
}
void loop()
{
    delay(2000); // time to put the robot down / get ready

    Serial.println("1 Forward");
    moveForward(ONE_METER_TIME);  // 1. Forward 100 cm
    Serial.println("2 Backward");
    moveBackward(ONE_METER_TIME); // 2. Backward 100 cm
    Serial.println("3 Turn Left");
    turnLeft(TURN_90_TIME);       // 3. Turn left 90°
    Serial.println("4 Forward");
    moveForward(ONE_METER_TIME);  // 4. Forward 100 cm
    Serial.println("5 Turn Right");
    turnRight(TURN_90_TIME);      // 5. Turn right 90°
    Serial.println("6 Forward");
    moveForward(ONE_METER_TIME);  // 6. Forward 100 cm

}
