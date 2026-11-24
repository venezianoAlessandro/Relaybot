
#include <Arduino.h>

// =====================================
// SENSORS
// =====================================

const int SENSOR_COUNT = 8;

int lineSensors[SENSOR_COUNT] = {
    A7, A6, A5, A4, A3, A2, A1, A0
};

// Sensor weights: left to right
int sensorWeights[SENSOR_COUNT] = {
    -7, -5, -3, -1, 1, 3, 5, 7
};

// Individual sensor thresholds
int thresholds[SENSOR_COUNT] = {
    645,  // A7
    694,  // A6
    654,  // A5
    632,  // A4
    587,  // A3
    606,  // A2
    535,  // A1
    510   // A0
};

int sensorValues[SENSOR_COUNT];
int sensorStates[SENSOR_COUNT];

// =====================================
// MOTOR PINS
// =====================================

const byte RIGHT_FORWARD = 3;
const byte RIGHT_BACKWARD = 5;
const byte LEFT_FORWARD = 6;
const byte LEFT_BACKWARD = 9;

// Calibrated motor speeds
const int RIGHT_BASE_SPEED = 170;
const int LEFT_BASE_SPEED = 225;

// =====================================
// STEERING SETTINGS
// =====================================

const float CENTER_OFFSET = 2.0;

// Normal steering
const float Kp = 20.0;

// Sharp-turn steering
const float SHARP_KP = 65.0;

// Sensor filtering
const float FILTER_STRENGTH = 0.25;
const float SHARP_FILTER_STRENGTH = 0.85;

// Sharp-turn detection duration
const unsigned long SHARP_SENSITIVITY_TIME = 700;
unsigned long sharpTurnUntil = 0;

// Line position and calculations
float linePosition = 0;
float filteredError = 0;

float error = 0;
float correction = 0;

// Motor speeds
int rightMotorSpeed = 0;
int leftMotorSpeed = 0;

const int MAX_SPEED = 255;
const int MIN_SPEED = 90;

const int STRAIGHT_BOOST = 15;

// Remember last direction
int lastLineDirection = 0;

// Recovery settings
const unsigned long GAP_GRACE_TIME = 200;
unsigned long lostLineTime = 0;

// Sharper recovery turn
const int SEARCH_FAST = 240;
const int SEARCH_SLOW = 70;

// =====================================
// ALL-BLACK STOP DETECTION
// =====================================

const unsigned long ALL_BLACK_STOP_TIME = 200;

unsigned long allBlackStartTime = 0;

bool allBlackTimerRunning = false;
bool stoppedOnAllBlack = false;
// =====================================
// REVERSE AFTER STOP
// =====================================

const unsigned long REVERSE_DELAY = 200;
const unsigned long REVERSE_TIME = 1000;

unsigned long stopTime = 0;

bool reverseStarted = false;
bool reverseFinished = false;

// =====================================
// MOTOR CONTROL
// =====================================

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

// =====================================
// READ SENSORS
// =====================================

void readSensors()
{
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        sensorValues[i] = analogRead(lineSensors[i]);

        sensorStates[i] =
            sensorValues[i] > thresholds[i] ? 1 : 0;
    }
}

// =====================================
// CALCULATE LINE POSITION
// =====================================

float calculatePosition()
{
    int total = 0;
    int activeSensors = 0;

    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        if (sensorStates[i] == 1)
        {
            total += sensorWeights[i];
            activeSensors++;
        }
    }

    // No black detected
    if (activeSensors == 0)
    {
        return 999;
    }

    // All black: treat as a marker
    if (activeSensors == SENSOR_COUNT)
    {
        return CENTER_OFFSET;
    }

    return (float)total / activeSensors;
}

// =====================================
// FOLLOW LINE
// =====================================

void followLine()
{
    error = linePosition - CENTER_OFFSET;

    // Detect a sharp turn
    if (error <= -2.0 || error >= 2.0)
    {
    sharpTurnUntil = millis() + SHARP_SENSITIVITY_TIME;
    }

    // Check whether sharp-turn mode is active
    bool sharpMode = (long)(sharpTurnUntil - millis()) > 0;

    // React faster during sharp turns
    float filterStrength = sharpMode
        ? SHARP_FILTER_STRENGTH
        : FILTER_STRENGTH;

    filteredError += filterStrength *
                     (error - filteredError);

    // Stronger steering during sharp turns
    float steeringStrength = sharpMode
        ? SHARP_KP
        : Kp;

    correction = steeringStrength * filteredError;
    // Extra correction for right turns
    if (filteredError > 2.0)
    {
        correction *= 1.25;
    }

    // Remember the last direction
    if (filteredError < -0.5)
    {
        lastLineDirection = -1;
    }
    else if (filteredError > 0.5)
    {
        lastLineDirection = 1;
    }

    // Speed boost only on straight sections
    int boost = 0;

    if (filteredError > -1.0 &&
        filteredError < 1.0 &&
        !sharpMode)
    {
        boost = STRAIGHT_BOOST;
    }

    // Calculate motor speeds
    rightMotorSpeed =
        RIGHT_BASE_SPEED + boost - correction;

    leftMotorSpeed =
        LEFT_BASE_SPEED + boost + correction;

    // Allow tighter turns without reversing
    rightMotorSpeed = constrain(
        rightMotorSpeed, MIN_SPEED, MAX_SPEED
    );

    leftMotorSpeed = constrain(
        leftMotorSpeed, MIN_SPEED, MAX_SPEED
    );

    setMotor(
        RIGHT_FORWARD,
        RIGHT_BACKWARD,
        rightMotorSpeed
    );

    setMotor(
        LEFT_FORWARD,
        LEFT_BACKWARD,
        leftMotorSpeed
    );
}

// =====================================
// RECOVER LOST LINE
// =====================================

void recoverLine()
{
    if (lostLineTime == 0)
    {
        lostLineTime = millis();
    }

    // Briefly continue previous movement
    if (millis() - lostLineTime < GAP_GRACE_TIME)
    {
        return;
    }

    // Sharper recovery in last known direction
    if (lastLineDirection < 0)
    {
        rightMotorSpeed = SEARCH_FAST;
        leftMotorSpeed = SEARCH_SLOW;
    }
    else if (lastLineDirection > 0)
    {
        rightMotorSpeed = SEARCH_SLOW;
        leftMotorSpeed = SEARCH_FAST;
    }
    else
    {
        rightMotorSpeed = RIGHT_BASE_SPEED;
        leftMotorSpeed = LEFT_BASE_SPEED;
    }

    setMotor(RIGHT_FORWARD, RIGHT_BACKWARD, rightMotorSpeed);
    setMotor(LEFT_FORWARD, LEFT_BACKWARD, leftMotorSpeed);
}

bool allSensorsBlack()
{
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        if (sensorStates[i] == 0)
        {
            return false;
        }
    }

    return true;
}

// =====================================
// SETUP
// =====================================

void setup()
{
    Serial.begin(9600);

    pinMode(RIGHT_FORWARD, OUTPUT);
    pinMode(RIGHT_BACKWARD, OUTPUT);
    pinMode(LEFT_FORWARD, OUTPUT);
    pinMode(LEFT_BACKWARD, OUTPUT);

    stopMotors();
}

// =====================================
// MAIN LOOP
// =====================================

void loop()
{
    // If the robot has finished reversing, remain stopped
    if (reverseFinished)
    {
        stopMotors();
        return;
    }

    // If the all-black stop condition was triggered
    if (stoppedOnAllBlack)
    {
        // Wait briefly before reversing
        if (!reverseStarted)
        {
            if (millis() - stopTime >= REVERSE_DELAY)
            {
                // Reverse both motors
                setMotor(
                    RIGHT_FORWARD,
                    RIGHT_BACKWARD,
                    -RIGHT_BASE_SPEED
                );

                setMotor(
                    LEFT_FORWARD,
                    LEFT_BACKWARD,
                    -LEFT_BASE_SPEED
                );

                reverseStarted = true;
                stopTime = millis();
            }

            return;
        }

        // Reverse for one second
        if (millis() - stopTime >= REVERSE_TIME)
        {
            stopMotors();
            reverseFinished = true;
        }

        return;
    }

    readSensors();

    // Check whether all sensors detect black
    if (allSensorsBlack())
    {
        if (!allBlackTimerRunning)
        {
            allBlackStartTime = millis();
            allBlackTimerRunning = true;
        }

        if (millis() - allBlackStartTime >= ALL_BLACK_STOP_TIME)
        {
            stopMotors();

            stoppedOnAllBlack = true;
            stopTime = millis();

            return;
        }
    }
    else
    {
        allBlackTimerRunning = false;
    }

    linePosition = calculatePosition();

    if (linePosition == 999)
    {
        recoverLine();
    }
    else
    {
        lostLineTime = 0;
        followLine();
    }
}