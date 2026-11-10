#include <Arduino.h>
const int MOTOR_A_1 = 3; // Pin for Motor A
const int MOTOR_A_2 = 5; // Pin for Motor A

void setup() {
  // Initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  pinMode(MOTOR_A_1,OUTPUT); // left motor
  pinMode(MOTOR_A_2,OUTPUT); // right motor 
}

void loop() {
  // Your main code here
  //speed forward motion
  for(int pwn = 0; pwn < 256; pwn++) {
    analogWrite(MOTOR_A_1, 0); // Set pin 10 to HIGH (full speed)
    analogWrite(MOTOR_A_2, pwn); // Set pin 11 to HIGH (full speed)
    if (millis() < 1000) { // added a function to millis() to wait for 1 second before the next iteration
      delay(1000); // wait for 1 second 
    }
  }
  analogWrite(MOTOR_A_1, 0); // Set pin 10 to HIGH (full speed)
  analogWrite(MOTOR_A_2, 0); // Set pin 11 to HIGH (full speed)

  //stop
  analogWrite(MOTOR_A_1, 0); // Set pin 10 to LOW (stop)
  analogWrite(MOTOR_A_2, 0); // Set pin 11 to LOW (stop)
//wait 10 seconds
  if (millis() < 1000) { // added a function to millis() to wait for 1 second before the next iteration
    delay(1000); // wait for 1 second
  }
}

void motorBackward(int speed) {
  analogWrite(MOTOR_A_1 , speed); // Set pin 10 to HIGH (full speed)
  analogWrite(MOTOR_A_2, 0); // Set pin 11 to LOW (stop)
}
void motorForward(int speed) {
  analogWrite(MOTOR_A_1, 0); // Set pin 10 to LOW (stop)
  analogWrite(MOTOR_A_2, speed); // Set pin 11 to HIGH (full speed)
}
void motorStop() {
  analogWrite(MOTOR_A_1, 0); // Set pin 10 to LOW (stop)
  analogWrite(MOTOR_A_2, 0); // Set pin 11 to LOW (stop)
}
