#include <Arduino.h>
#define gripper 11

int motorOneSpeed = 0;
int motorTwoSpeed = 0;

enum LineState { LINE_NONE, LINE_LEFT, LINE_RIGHT, LINE_CENTER, LINE_T, LINE_END };

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
//line
LineState readLine(void);
void handleLine(LineState);


//A1 and B1 are speed
//A2 and B2 are direction

//motor 1 is left, motor 2 is right

const int motorSpeed1 = 10;
const int motorSpeed2 = 6;
const int motorSpeedReverse1 = 9;
const int motorSpeedReverse2 = 5;
const int motor1 = 9;
const int motor2 = 5;

const int speedcheck1 = 2; //https://thewanderingengineer.com/2014/08/11/arduino-pin-change-interrupts/
const int speedcheck2 = 3;

float RPM = 0;

long timer;
long timer2;

int distance;

const int trigger = 4;
const int echo = 7;

const int sensorCount = 8;
const int sensorPins[sensorCount] = {A0, A1, A2, A3, A4, A5, 3, 11};
int sensorValues[sensorCount];

const int BLACK = 900;

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

  for(int i=0; i<sensorCount; i++) 
    pinMode(sensorPins[i], INPUT);

  digitalWrite(gripper, HIGH);  
    delay(1);
    printf("here");
  digitalWrite(gripper, LOW);
}

void loop() {
  howLong();
  while (millis() <= timer + 3000){
    handleLine(readLine());
    if (millis() <= timer2 + 500){
      howLong();
      object_Avoidence();
    }
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
    timer2 = millis();
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
  distance = 0;
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  distance = (.034 * pulseIn(echo, HIGH, 5900))/2;
  if(distance == 0){
    return;
  }
  else if(distance < 10){
    howLong();
    while (millis() <= timer2 + 500){
      motorSpeedAdjuster(motor1, 1, true);
      motorSpeedAdjuster(motor2, .9, false);
    }
    howLong();
    while (millis() <= timer2 + 300){
      motorSpeedAdjuster(motor1, 1, true);
      motorSpeedAdjuster(motor2, .7, true);
    }
    howLong();
    while (millis() <= timer2 + 500){
      motorSpeedAdjuster(motor1, 1, false);
      motorSpeedAdjuster(motor2, .7, true);
    }
    stop();
    object_Avoidence();
    howLong();
    return;
  }
  else{
    return;
  }
}

LineState readLine() {
  for(int i=0; i<sensorCount; i++)
    sensorValues[i] = analogRead(sensorPins[i]);

  bool right = sensorValues[0] > BLACK || sensorValues[1] > BLACK || sensorValues[2] > BLACK;
  bool center = sensorValues[3] > BLACK || sensorValues[4] > BLACK;
  bool left = sensorValues[5] > BLACK || sensorValues[6] > BLACK || sensorValues[7] > BLACK;

  if(left && center && right) 
    return LINE_T;
  else if(!left && !center && !right)
    return LINE_END;
  else if(center) 
    return LINE_CENTER;
  else if(left) 
    return LINE_LEFT;
  else if(right) 
    return LINE_RIGHT;
  else
    return LINE_NONE;
}

void handleLine(LineState line) {
  if(line == LINE_CENTER){
    motorSpeedAdjuster(motor1, .95, true);
    motorSpeedAdjuster(motor2, 1, true);
  }
  else if (line == LINE_LEFT)
  {
    motorSpeedAdjuster(motor1, .6, false);
    motorSpeedAdjuster(motor2, 1, true);
  }
  else if (line == LINE_RIGHT){
    motorSpeedAdjuster(motor1, .95, true);
    motorSpeedAdjuster(motor2, .6, false);
  }
  else if (line == LINE_T){
    motorSpeedAdjuster(motor1, .95, true);
    motorSpeedAdjuster(motor2, 1, true);
  }
  else if (line == LINE_END){
    motorSpeedAdjuster(motor1, .95, true);
    motorSpeedAdjuster(motor2, 1, true);
  }
}


