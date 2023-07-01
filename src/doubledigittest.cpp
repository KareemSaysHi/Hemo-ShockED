#include <Arduino.h>


// DOUBLE DIGIT 7 SEGMENT PINS
// NOTE: the latch pin is active on rising edge, so you have to switch it off and on when you are changing data
int data = 8;
int clock = 10;
int latch = 9;
int digit1 = 6; //digit 1 is the MSB
int digit2 = 7;
int dataButton = 2;
int clockButton = 3;
int Digits[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 103}; //digits encoded in binary

unsigned long onTime = 3; //how long each 7seg is on for and off for

int cycleTracker = 0; // for the function display7seg
int cycleState = 0;

unsigned long i = 0;
unsigned long counter = 0;



unsigned long previousMillis = 0;

void display7seg (int number) {
    if (millis()-previousMillis >= onTime) {
        cycleState = (cycleState + 1) % 4;
        previousMillis = millis();
        if (cycleState == 0) {
            digitalWrite(digit1, LOW);
            digitalWrite(digit2, HIGH);

            digitalWrite(latch, LOW);
            shiftOut(data, clock, MSBFIRST, Digits[number % 10]);
            digitalWrite(latch, HIGH);
        } else if (cycleState == 1) {
            digitalWrite(latch, LOW);
            shiftOut(data, clock, MSBFIRST, 0);
            digitalWrite(latch, HIGH);
        } else if (cycleState == 2) {
            digitalWrite(digit1, HIGH);
            digitalWrite(digit2, LOW);

            digitalWrite(latch, LOW);
            shiftOut(data, clock, MSBFIRST, Digits[int(number / 10)]);
            digitalWrite(latch, HIGH);
        } else {
            digitalWrite(latch, LOW);
            shiftOut(data, clock, MSBFIRST, 0);
            digitalWrite(latch, HIGH);
        }
    }
}


void update7seg (int number) {
    return;
}

void setup() {
  pinMode(dataButton, INPUT_PULLUP);
  pinMode(clockButton, INPUT_PULLUP);

  pinMode(latch, OUTPUT);
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);

  pinMode(digit1, OUTPUT);
  pinMode(digit2, OUTPUT);

  Serial.begin(115200);
  digitalWrite(latch, HIGH);

  
}

void loop() {
    display7seg(i);
    counter ++;
    if (millis() > i*2000) {
        Serial.println(counter);
        counter = 0;
        i++;
    }
}
    


  

