#include <Arduino.h>
#include <Adafruit_NeoPixel.h> // NeoPixel library

#define PIN 13
#define NUM_PIXELS 4
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

const int motorPin1 = 3;
const int motorPin2 = 5;
const int motorPin3 = 6;
const int motorPin4 = 9;

const int servoPin = 10;

const int sensorCount = 8;
const int sensorPins[sensorCount] = {A0, A1, A2, A3, A4, A5, A6, A7};
int sensorValues[sensorCount];

const int BLACK = 900;

unsigned long timeStarted = 0;
bool timerStarted = false;
bool raceStarted = false;
bool raceEnded = false;

int celebrationStep = 0;
unsigned long timePassed = 0;
unsigned long timeCelebration = 0;

//====================================//
// ENUMS
//====================================//
enum LineState { LINE_NONE, LINE_LEFT, LINE_RIGHT, LINE_CENTER, LINE_T, LINE_END };
enum TurnState { TURN_NONE, TURN_LEFT, TURN_RIGHT, TURN_AROUND };
TurnState turnState = TURN_NONE;

// ===== Function prototypes =====
void startRace();
void endRace();
void startTimer();
void moveGripper(int angle);

void stopRobot();
void lightStop();
void moveForward();
void moveBackwards();
void turnRight();
void moveRight();
void moveLeft();
void turnAround();

void celebrate1();
void celebrate2();
void celebrate3();
void celebrate4();

void readSensors();
LineState readLine();
void handleLine(LineState line);
void handleTurn();
void handleCelebration();


//====================================//
// SETUP
//====================================//
void setup() {
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  pinMode(servoPin, OUTPUT);

  for(int i=0; i<sensorCount; i++) pinMode(sensorPins[i], INPUT);

  Serial.begin(9600);

  pixels.begin();
  pixels.show();
  pixels.setBrightness(50);

  delay(1000);
  startRace();
  startTimer();
  moveForward();
}

//====================================//
// LOOP
//====================================//
void loop() {
  // читаем сенсоры напрямую
  readSensors();

  // если гонка закончилась, делаем анимацию
  if(raceEnded) {
    handleCelebration();
    return;
  }

  // обрабатываем повороты через state-machine
  if(turnState != TURN_NONE) {
    handleTurn();
    return;
  }

  // обработка линии каждый цикл
  LineState line = readLine();
  handleLine(line);
}

//====================================//
// SENSOR READING
//====================================//
void readSensors() {
  for(int i=0; i<sensorCount; i++){
    sensorValues[i] = analogRead(sensorPins[i]);
  }
}

//====================================//
// LINE READING
//====================================//
LineState readLine() {
  bool right = sensorValues[0] > BLACK || sensorValues[1] > BLACK || sensorValues[2] > BLACK;
  bool center = sensorValues[3] > BLACK || sensorValues[4] > BLACK;
  bool left = sensorValues[5] > BLACK || sensorValues[6] > BLACK || sensorValues[7] > BLACK;

  if(left && center && right) return LINE_T;
  if(!left && !center && !right) return LINE_END;
  if(center) return LINE_CENTER;
  if(left) return LINE_LEFT;
  if(right) return LINE_RIGHT;

  return LINE_NONE;
}

//====================================//
// LINE HANDLING
//====================================//
void handleLine(LineState line) {
  switch(line) {
    case LINE_CENTER: moveForward(); break;
    case LINE_LEFT: turnState = TURN_LEFT; break;
    case LINE_RIGHT: turnState = TURN_RIGHT; break;
    case LINE_T: turnState = TURN_RIGHT; break; // или по твоей логике налево
    case LINE_END: turnState = TURN_AROUND; break;
    default: turnState = TURN_AROUND; break;
  }
}

//====================================//
// TURN HANDLING
//====================================//
void handleTurn() {
  switch(turnState) {
    case TURN_LEFT: moveLeft(); break;
    case TURN_RIGHT: moveRight(); break;
    case TURN_AROUND: turnAround(); break;
    default: break;
  }
  turnState = TURN_NONE;
}

//====================================//
// CELEBRATION
//====================================//
void handleCelebration() {
  unsigned long timeNow = millis();
  if(timeNow - timePassed >= timeCelebration){
    timePassed = timeNow;
    switch(celebrationStep){
      case 0: lightStop(); timeCelebration=2000; break;
      case 1: celebrate1(); timeCelebration=200; break;
      case 2: celebrate2(); timeCelebration=200; break;
      case 3: celebrate3(); timeCelebration=200; break;
      case 4: celebrate4(); timeCelebration=100; break;
      case 5: stopRobot(); celebrationStep=0; return;
    }
    celebrationStep++;
  }
}

//====================================//
// BASIC FUNCTIONS
//====================================//
void startRace(){ if(!raceStarted) raceStarted=true; }
void endRace(){ raceEnded=true; }
void startTimer(){ if(!timerStarted){ timeStarted=millis(); timerStarted=true; } }
void moveGripper(int angle){ analogWrite(servoPin, map(angle,0,180,0,255)); }

void stopRobot() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW);
}

void lightStop(){
  for(int i=0;i<NUM_PIXELS;i++) pixels.setPixelColor(i, pixels.Color(0,255,0));
  pixels.show();
}

void moveForward() {
  analogWrite(motorPin1,235); digitalWrite(motorPin2,LOW);
  analogWrite(motorPin3,255); digitalWrite(motorPin4,LOW);
  pixels.clear();
  pixels.setPixelColor(2,pixels.Color(155,255,0));
  pixels.setPixelColor(3,pixels.Color(155,255,0));
  pixels.show();
}

void moveBackwards() { 
  digitalWrite(motorPin1,LOW); analogWrite(motorPin2,210);
  digitalWrite(motorPin3,LOW); analogWrite(motorPin4,248);
  pixels.clear();
  pixels.setPixelColor(0,pixels.Color(0,255,0));
  pixels.setPixelColor(1,pixels.Color(0,255,0));
  pixels.show();
}

void turnRight() {
  analogWrite(motorPin1,120); digitalWrite(motorPin2,LOW);
  digitalWrite(motorPin3,LOW); analogWrite(motorPin4,160);
  pixels.clear();
  pixels.setPixelColor(1,pixels.Color(155,255,0));
  pixels.setPixelColor(2,pixels.Color(155,255,0));
  pixels.show();
}

void moveRight() {
  analogWrite(motorPin1,255); digitalWrite(motorPin2,LOW);
  analogWrite(motorPin3,LOW); digitalWrite(motorPin4,LOW);
  pixels.clear();
  pixels.setPixelColor(2,pixels.Color(155,255,0));
  pixels.show();
}

void moveLeft() {
  analogWrite(motorPin1,LOW); digitalWrite(motorPin2,LOW);
  analogWrite(motorPin3,255); digitalWrite(motorPin4,LOW);
  pixels.clear();
  pixels.setPixelColor(3,pixels.Color(155,255,0));
  pixels.show();
}

void turnAround() {
  digitalWrite(motorPin1,LOW); analogWrite(motorPin2,170);
  analogWrite(motorPin3,130); digitalWrite(motorPin4,LOW);
  pixels.clear();
  pixels.setPixelColor(0,pixels.Color(155,255,0));
  pixels.setPixelColor(3,pixels.Color(155,255,0));
  pixels.show();
}

//====================================//
// CELEBRATION PIXELS
//====================================//
void celebrate1(){ 
  pixels.setPixelColor(0,pixels.Color(0,70,0));
  pixels.setPixelColor(1,pixels.Color(0,255,0));
  pixels.setPixelColor(2,pixels.Color(0,70,0));
  pixels.setPixelColor(3,pixels.Color(0,70,0));
  pixels.show();
}

void celebrate2(){ 
  pixels.setPixelColor(0,pixels.Color(0,70,0));
  pixels.setPixelColor(1,pixels.Color(0,70,0));
  pixels.setPixelColor(2,pixels.Color(0,255,0));
  pixels.setPixelColor(3,pixels.Color(0,70,0));
  pixels.show();
}

void celebrate3(){ 
  pixels.setPixelColor(0,pixels.Color(0,70,0));
  pixels.setPixelColor(1,pixels.Color(0,70,0));
  pixels.setPixelColor(2,pixels.Color(0,70,0));
  pixels.setPixelColor(3,pixels.Color(0,255,0));
  pixels.show();
}

void celebrate4(){ 
  pixels.setPixelColor(0,pixels.Color(0,255,0));
  pixels.setPixelColor(1,pixels.Color(0,70,0));
  pixels.setPixelColor(2,pixels.Color(0,70,0));
  pixels.setPixelColor(3,pixels.Color(0,70,0));
  pixels.show();
}

// test
