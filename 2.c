#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// ================================
// NEOPIXEL
// ================================
#define PIN 13
#define NUM_PIXELS 4
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

// ================================
// MOTORS
// ================================
const int motorPin1 = 3;
const int motorPin2 = 5;
const int motorPin3 = 6;
const int motorPin4 = 9;

// ================================
// GRIPPER
// ================================
const int gripperPin = 10;

// ================================
// SENSORS
// ================================
const int sensorCount = 8;
const int sensorPins[sensorCount] = {A0,A1,A2,A3,A4,A5,A6,A7};
int sensorValues[sensorCount];

const int BLACK = 900;

// ================================
// STATE
// ================================
bool inTurn = false;
int lastDirection = 0; // 1=right, -1=left, 0=forward
int rightDetectCount = 0; // счётчик обнаружения правого поворота
int turnCommitCount = 0;  // счётчик итераций поворота (для форсирования)

// ================================
// BLACK SQUARE DETECTION
// ================================
unsigned long allBlackStartTime = 0;
const unsigned long BLACK_SQUARE_THRESHOLD = 100; // время в мс (300-500мс)
bool finishDetected = false;

// ================================
// PROTOTYPES
// ================================
void readSensors();
void handleNavigation();
void moveForward();
void moveBackward();
void moveLeft();
void moveRight();
void stopRobot();
void startSequence();
void finishSequence();
void setGripperPulse(int pulseWidthMicros);
void closeGripper();
void openGripper();
void Uturn();

// ================================
// SETUP
// ================================
void setup() {
    pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);
    pinMode(motorPin3, OUTPUT);
    pinMode(motorPin4, OUTPUT);
    pinMode(gripperPin, OUTPUT);

    for(int i=0; i<sensorCount; i++)
        pinMode(sensorPins[i], INPUT);

    Serial.begin(9600);

    pixels.begin();
    pixels.setBrightness(40);
    pixels.show();

    startSequence();
}

// ================================
// LOOP
// ================================
void loop() {
    // Если финиш обнаружен - больше ничего не делаем
    if(finishDetected) {
        return;
    }
    
    readSensors();
    handleNavigation();
}

// ================================
// SENSOR READ
// ================================
void readSensors() {
    for(int i=0; i<sensorCount; i++)
        sensorValues[i] = analogRead(sensorPins[i]);
}

// ================================
// SIMPLE RIGHT-HAND NAVIGATION
// ================================
void handleNavigation() {
    // Читаем сенсоры
    bool right  = sensorValues[0]>BLACK || sensorValues[1]>BLACK || sensorValues[2]>BLACK;
    bool center = sensorValues[3]>BLACK || sensorValues[4]>BLACK;
    bool left   = sensorValues[5]>BLACK || sensorValues[6]>BLACK || sensorValues[7]>BLACK;
    
    bool anyLine = right || center || left;
    
    // === ПРОВЕРКА ЧЁРНОГО КВАДРАТА (ВСЕ СЕНСОРЫ ЧЁРНЫЕ) ===
    bool allBlack = right && center && left && 
                    sensorValues[0]>BLACK && sensorValues[1]>BLACK && 
                    sensorValues[2]>BLACK && sensorValues[3]>BLACK && 
                    sensorValues[4]>BLACK && sensorValues[5]>BLACK && 
                    sensorValues[6]>BLACK && sensorValues[7]>BLACK;
    
    if(allBlack) {
        if(allBlackStartTime == 0) {
            allBlackStartTime = millis();
        } else if(millis() - allBlackStartTime >= BLACK_SQUARE_THRESHOLD) {
            finishDetected = true;
            finishSequence();
            return;
        }
    } else {
        allBlackStartTime = 0;
    }
    
    // === ЕСЛИ В ПОВОРОТЕ ===
    if(inTurn) {
        turnCommitCount++;
        
        // ФОРСИРУЕМ поворот минимум 15-20 итераций (чтобы точно свернуть)
        if(turnCommitCount < 20) {
            // Принудительно крутимся, игнорируя сенсоры
            if(lastDirection == 1)
                moveRight();
            else if(lastDirection == -1)
                moveLeft();
            return;
        }
        
        // После 20 итераций ждём центральную линию
        if(center && !right && !left) {
            inTurn = false;
            rightDetectCount = 0;
            turnCommitCount = 0;
            moveForward();
            lastDirection = 0;
        } else {
            if(lastDirection == 1)
                moveRight();
            else if(lastDirection == -1)
                moveLeft();
        }
        return;
    }
    
    // === ОБЫЧНЫЙ РЕЖИМ - ПРАВИЛО ПРАВОЙ РУКИ ===
    
    // Проверяем правую сторону
    if(right) {
        rightDetectCount++;
        // Фиксируем поворот после 2-3 обнаружений подряд
        if(rightDetectCount >= 2) {
            inTurn = true;
            lastDirection = 1;
            moveRight();
            return;
        }
    } else {
        rightDetectCount = 0; // сбрасываем если не видим право
    }
    
    // Если справа ничего нет, едем прямо
    if(center) {
        moveForward();
        lastDirection = 0;
        return;
    }
    
    // Если прямо тоже ничего, поворачиваем налево
    if(left) {
        inTurn = true;
        lastDirection = -1;
        moveLeft();
        return;
    }
    
    // Если вообще ничего не видим - разворот направо (правило правой руки)
    if(!anyLine) {
        Uturn();
    }
}

// ================================
// MOVEMENT
// ================================
void moveForward() {
    analogWrite(motorPin1, 221); digitalWrite(motorPin2, LOW);
    analogWrite(motorPin3, 255); digitalWrite(motorPin4, LOW);

    pixels.clear();
    pixels.setPixelColor(2, pixels.Color(255, 255, 255));
    pixels.setPixelColor(3, pixels.Color(255, 255, 255));
    pixels.show();
}

void moveBackward() {
    digitalWrite(motorPin1, LOW); analogWrite(motorPin2, 220);
    digitalWrite(motorPin3, LOW); analogWrite(motorPin4, 255);

    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.setPixelColor(1, pixels.Color(0, 255, 0));
    pixels.show();
}

void moveLeft() {
    analogWrite(motorPin1, LOW);   digitalWrite(motorPin2, LOW);
    analogWrite(motorPin3, 255); digitalWrite(motorPin4, LOW);

    pixels.clear();
    pixels.setPixelColor(3, pixels.Color(255, 0, 0));
    pixels.show();
}

void moveRight() {
    analogWrite(motorPin1, 255); digitalWrite(motorPin2, LOW);
    analogWrite(motorPin3, LOW);   digitalWrite(motorPin4, LOW);

    pixels.clear();
    pixels.setPixelColor(2, pixels.Color(255, 0, 0));
    pixels.show();
}

void Uturn() {
    analogWrite(motorPin1, 255); digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW); analogWrite(motorPin4, 255);

    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(255, 255, 0));
    pixels.setPixelColor(1, pixels.Color(255, 255, 0));
    pixels.setPixelColor(2, pixels.Color(255, 255, 0));
    pixels.setPixelColor(3, pixels.Color(255, 255, 0));
    pixels.show();
}

void stopRobot() {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
    
    pixels.clear();
    pixels.show();
}

// ================================
// FINISH SEQUENCE
// ================================
void finishSequence() {
    // Останавливаемся
    stopRobot();
    delay(300);
    
    // Мигаем всеми светодиодами (финиш!)
    for(int i=0; i<3; i++) {
        pixels.fill(pixels.Color(255, 0, 0));
        pixels.show();
        delay(200);
        pixels.clear();
        pixels.show();
        delay(200);
    }
    
    moveLeft();
    delay(175);
    
    stopRobot();
    delay(300);

    // Открываем грипер - оставляем груз
    openGripper();
    delay(500);
    
    // Отъезжаем назад
    moveBackward();
    delay(2000); // едем назад 1 секунду
    
    // Останавливаемся
    stopRobot();
    
    // Финальная индикация - синий цвет (завершено)
    pixels.fill(pixels.Color(0, 0, 255));
    pixels.show();
}

// ================================
// GRIPPER
// ================================
void setGripperPulse(int pulseWidthMicros) {
    for(int i = 0; i < 25; i++) {
        digitalWrite(gripperPin, HIGH);
        delayMicroseconds(pulseWidthMicros);
        digitalWrite(gripperPin, LOW);
        delayMicroseconds(20000 - pulseWidthMicros);
    }
}

void closeGripper() {
    setGripperPulse(1100);
}

void openGripper() {
    setGripperPulse(1900);
}

// ================================
// START SEQUENCE
// ================================
void startSequence() {
    delay(2000);

    moveForward();
    delay(750);
    stopRobot();
    delay(500);

    closeGripper();

    moveForward();
    delay(250);
    stopRobot();

    moveLeft();
    delay(750);
    stopRobot();

    moveForward();
}