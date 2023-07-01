#include <Arduino.h>

int data = 8;
int clock = 10;
int latch = 9;
int digit1 = 6;
int digit2 = 7;
int dataButton = 2;
int clockButton = 3;
int Digits[10] = {64, 121, 36, 48, 25, 18, 2, 120, 0, 24};
int Test[7] = {1, 2, 4, 8, 16, 32, 64};

int tens = 0;
int ones = 0;

void setup() {
  pinMode(dataButton, INPUT_PULLUP);
  pinMode(clockButton, INPUT_PULLUP);

  pinMode(latch, OUTPUT);
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);

  pinMode(digit1, OUTPUT);
  pinMode(digit2, OUTPUT);

  Serial.begin(9600);
  digitalWrite(latch, HIGH);
}

void loop() {
  for (int i = 0; i < 100; i++) {
    tens = int(i / 10);
    ones = i % 10;
    Serial.println(tens);
    Serial.println(ones);
    for (int j=0; j < 100; j++) {

      digitalWrite(digit1, LOW);
      digitalWrite(digit2, HIGH);

      digitalWrite(latch, LOW);
      shiftOut(data, clock, MSBFIRST, 255-128-Digits[tens]);
      digitalWrite(latch, HIGH);

      delay(5);

      digitalWrite(latch, LOW);
      shiftOut(data, clock, MSBFIRST, 0);
      digitalWrite(latch, HIGH);

      //digitalWrite(digit1, LOW);
      //digitalWrite(digit2, LOW);

      delay(5);

      digitalWrite(digit2, LOW);
      digitalWrite(digit1, HIGH);
      
      digitalWrite(latch, LOW);
      shiftOut(data, clock, MSBFIRST, 255-128-Digits[ones]);
      digitalWrite(latch, HIGH);

      delay(5);

      digitalWrite(latch, LOW);
      shiftOut(data, clock, MSBFIRST, 0);
      digitalWrite(latch, HIGH);

      //digitalWrite(digit1, LOW);
      //digitalWrite(digit2, LOW);

      delay(5);
    }
  }
}
    


  

  /*for (int i = 0; i < 10; i++) {
    Serial.println(i);
    digitalWrite(latch, LOW);
    shiftOut(data, clock, MSBFIRST, 255-128-Digits[i]);
    digitalWrite(latch, HIGH);
    delay(1000);
  }*/




/*
  for manual testing:
  digitalWrite(data, abs(1-digitalRead(dataButton)));
  digitalWrite(clock, abs(1-digitalRead(clockButton)));
  Serial.println(abs(1-digitalRead(dataButton)));
  Serial.println(abs(1-digitalRead(clockButton)));
  Serial.println();
  delay(100); 
*/