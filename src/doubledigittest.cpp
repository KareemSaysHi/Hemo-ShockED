#include <Arduino.h> //you don't need this if you're not using vscode
#include <RTClib.h> //rtc library
#include <SPI.h> //for i2c comms, needed for rtc library
#include <TM1637Display.h> //4 digit 7 segment library, USING THE ONE WITH NO DELAYS

// DOUBLE DIGIT 7 SEGMENT PINS
// NOTE: the latch pin is active on rising edge, so you have to switch it off and on when you are changing data

int latch = 22; 

//I'm using arrays to define pins for the 2 digit LEDs, since it'll turn out to be very very helpful for our display7seg function

int twoDigitPins[5][2] = {{33, 34}, {35, 36}, {37, 38}, {39, 40}, {41, 42}}; //This is an array inside an array!  I can explain this way more if you guys want

int datas[5] = {23, 24, 25, 26, 27}; //This is an array of the data pins for each of the modules
int clocks[5] = {28, 29, 30, 31, 32};
int counters[5] = {1, 2, 3, 4, 5};

int testingLed = 3;
int data = datas[testingLed];
int clock = clocks[testingLed];
int digit1 = twoDigitPins[testingLed][0]; //digit 1 is the MSB
int digit2 = twoDigitPins[testingLed][1];


int Digits[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 103}; //digits encoded in binary

// 24 45 44
// 25 46 47
// 26 48 49
// 27 51 50
// 28 53 52

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
    if (millis() > i*500) {
        Serial.println(i);
        counter = 0;
        i++;
    }
}
    


  

