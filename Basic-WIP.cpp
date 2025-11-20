#include <Arduino.h>
#define gripper 11

int motorOneSpeed = 0;
int motorTwoSpeed = 0;

//Motor Number, Speed is a percentage between 0 and 1, forward yes no
void motorSpeedAdjuster (int, float, boolean);
//stop
void stop (void);
//RPM
void rpmMachine (void);
//wait
long howLong(void);
//object
void object_Avoidence(void);

//A1 and B1 are speed
//A2 and B2 are direction

//motor 1 is left, motor 2 is right

int motorSpeed1 = 10;
int motorSpeed2 = 7;
int motorSpeedReverse1 = 9;
int motorSpeedReverse2 = 6;
int motor1 = 9;
int motor2 = 6;

int speedcheck1 = 2;
int speedcheck2 = 3;

float RPM = 0;

long timer;

int trigger = 4;
int echo = 5;

void setup() {
Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(speedcheck1), rpmMachine, INPUT_PULLUP);

  pinMode(motorSpeed1, OUTPUT);
  pinMode(motorSpeedReverse1, OUTPUT);
  pinMode(motorSpeed2, OUTPUT);
  pinMode(motorSpeedReverse2, OUTPUT);

  pinMode(speedcheck1, INPUT);
  pinMode(speedcheck2, INPUT);
  stop();

  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(gripper, OUTPUT);

  digitalWrite(gripper, HIGH);  
    delay(1);
    printf("here");
  digitalWrite(gripper, LOW);
}

void loop() {
  howLong();
  while (millis() <= timer + 3000){
    motorSpeedAdjuster(motor1, .95, true);
    motorSpeedAdjuster(motor2, .7, true);
    object_Avoidence();
  }
  howLong();
  while (millis() <= timer + 4000){
    motorSpeedAdjuster(motor1, .7, false);
    motorSpeedAdjuster(motor2, .84, false);
  }
  howLong();
  while (millis() <= timer + 1000){
    stop();
  }
  howLong();
  while (millis() <= timer + 500){
    motorSpeedAdjuster(motor1, 1, true);
    motorSpeedAdjuster(motor2, 0, true);
  }
  howLong();
  while (millis() <= timer + 1000){
    stop();
  }
  howLong();
  while (millis() <= timer + 3000){
    motorSpeedAdjuster(motor1, .6, true);
    motorSpeedAdjuster(motor2, 1, true);
  }
  howLong();
  while (millis() <= timer + 1000){
    motorSpeedAdjuster(motor1, 1, true);
    motorSpeedAdjuster(motor2, .7, false);
  }
  howLong();
  while (millis() <= timer + 1000){ 
    stop();
  }
}

// put function definitions here:
void motorSpeedAdjuster (int Number, float Speed, boolean Forward) {
  int AnalogSpeed = 255 * Speed;

  if(Speed < .6){
    for (int sauce = 1; sauce < Speed; sauce -= .1)
    {
      AnalogSpeed = 255 * sauce;
      if(Forward){
        analogWrite(Number, 0);
        analogWrite(Number + 1, AnalogSpeed);
      }

      else{
        analogWrite(Number + 1, 0);
        analogWrite(Number, AnalogSpeed);
      }
      howLong();
      while (millis() <= timer + 5){}
    }
  }

  if(Forward){
    analogWrite(Number, 0);
    analogWrite(Number + 1, AnalogSpeed);
  }

  else{
    analogWrite(Number + 1, 0);
    analogWrite(Number, AnalogSpeed);
  }
}

long howLong(void){
  if(millis() >= timer){
    timer = millis();
  }
}

void stop (void){
  analogWrite(motorSpeed1, 0);
  analogWrite(motorSpeedReverse1, 0);
  analogWrite(motorSpeed2, 0);
  analogWrite(motorSpeedReverse2, 0);
}

void rpmMachine (void){
  static long RPMTimer;
  static long RPMTimer2;
}

void object_Avoidence(void){
  int distance = 0;
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  distance = (.034 * pulseIn(echo, HIGH, 5900))/2;
  if(distance < 20){
      digitalWrite(gripper, HIGH);  
      delay(1.5);
      printf("there");
      digitalWrite(gripper, LOW);
    howLong();
    while (millis() <= timer + 500){
      motorSpeedAdjuster(motor1, 1, true);
      motorSpeedAdjuster(motor2, .7, false);
    }
    howLong();
    while (millis() <= timer + 1000){
      motorSpeedAdjuster(motor1, .95, true);
      motorSpeedAdjuster(motor2, .7, true);
    }
    howLong();
    while (millis() <= timer + 500){
      motorSpeedAdjuster(motor1, .7, false);
      motorSpeedAdjuster(motor2, 1, false);
    }
    digitalWrite(gripper, HIGH);  
      delay(1);  
      printf("everywhere");
    digitalWrite(gripper, LOW);
    return;
  }
  return;
}
