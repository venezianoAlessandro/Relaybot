#include <Arduino.h>

// Motor pins
const byte RIGHT_FORWARD = 3;
const byte RIGHT_BACKWARD = 5;
const byte LEFT_FORWARD = 6;
const byte LEFT_BACKWARD = 9;

// Separate speeds so the robot drives straight (0-255)
// Right wheel is stronger, so it gets a lower number
const byte RIGHT_SPEED = 200;
const byte LEFT_SPEED = 225;

// Kick-start: both motors get full power briefly so they break free together
const byte KICK_PWM = 255;
const unsigned int KICK_TIME = 20; // ms, try 20-80

const unsigned int ONE_METER_TIME = 3000; // ms to travel the distance

// Each turn has its own speeds and duration
struct Turn
{
    int rightSpeed;
    int leftSpeed;
    unsigned int duration;
};

// Left turn: calibrated and working
const Turn TURN_LEFT_90  = {  RIGHT_SPEED, -LEFT_SPEED, 1300 };

// Right turn: starts longer because it was falling short.
// 1300 * (225/200) ~= 1460 is just a starting guess. Calibrate it (see below).
const Turn TURN_RIGHT_90 = { -RIGHT_SPEED,  LEFT_SPEED, 1460 };

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

// Runs both motors for a set time, then stop
void drive(int rightSpeed, int leftSpeed, unsigned int duration)
{
    // Kick-start both motors at the same moment, in the right direction
    setMotor(RIGHT_FORWARD, RIGHT_BACKWARD, rightSpeed > 0 ? KICK_PWM : -KICK_PWM);
    setMotor(LEFT_FORWARD, LEFT_BACKWARD,   leftSpeed  > 0 ? KICK_PWM : -KICK_PWM);
    delay(KICK_TIME);

    // Drop to the trimmed speeds
    setMotor(RIGHT_FORWARD, RIGHT_BACKWARD, rightSpeed);
    setMotor(LEFT_FORWARD, LEFT_BACKWARD, leftSpeed);
    delay(duration > KICK_TIME ? duration - KICK_TIME : 0);

    stopMotors();
    delay(2000); // 2s pause so the robot settles before the next move
}

void moveForward(unsigned int duration)  { drive( RIGHT_SPEED,  LEFT_SPEED, duration); }
void moveBackward(unsigned int duration) { drive(-RIGHT_SPEED, -LEFT_SPEED, duration); }
void turn(const Turn &t)                 { drive(t.rightSpeed, t.leftSpeed, t.duration); }

void setup()
{
    Serial.begin(9600);
    Serial.println("BOOT");

    pinMode(RIGHT_FORWARD, OUTPUT);
    pinMode(RIGHT_BACKWARD, OUTPUT);
    pinMode(LEFT_FORWARD, OUTPUT);
    pinMode(LEFT_BACKWARD, OUTPUT);

    stopMotors();
}

void loop()
{
    delay(1500);

    Serial.println("1 Forward");
    moveForward(ONE_METER_TIME);
    Serial.println("2 Backward");
    moveBackward(ONE_METER_TIME);
    Serial.println("3 Turn Left");
    turn(TURN_LEFT_90);
    Serial.println("4 Forward");
    moveForward(ONE_METER_TIME);
    Serial.println("5 Turn Right");
    turn(TURN_RIGHT_90);
    Serial.println("6 Forward");
    moveForward(ONE_METER_TIME);

    while (true) { stopMotors(); }
}