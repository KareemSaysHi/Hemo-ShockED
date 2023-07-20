//A test to see if I can make these led segments work :|

#include <Arduino.h> //you don't need this if you're not using vscode
#include <RTClib.h> //rtc library
#include <SPI.h> //for i2c comms, needed for rtc library
//#include <TM1637Display.h> //4 digit 7 segment library, USING THE ONE WITH NO DELAYS

//Latch and clock are shared between all shift registers
int latch = 22; 

const int numDigits = 5;

int twoDigitPins[5][2] = {{33, 34}, {35, 36}, {38, 37}, {39,40}, {41, 42}}; //This is an array inside an array!  I can explain this way more if you guys want

int data[5] = {23, 24, 25, 26, 27}; //This is an array of the data pins for each of the modules
int clocks[5] = {28, 29, 30, 31, 32};
int counters[5] = {01, 02, 03, 04, 05};

int Digits[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 103}; //digits encoded in binary, for a common anode display?

//These variables are for the display7seg function
unsigned long onTime = 5; //how long each of the 2d segments is on and off for
int cycleState[5] = {0, 0, 0, 0, 0}; //similar to the previous variable
int alreadyUpdated[5] = {0, 0, 0, 0, 0};
unsigned long previousMillis = 0; //unsigned longs are big numbers, which is good since they're gonna be storing large values


int buttons[5][2] = {{19,18}, {17,16}, {15,14}, {2,3}, {4,5}};
int change = 0;
int currentState[5][2] = {{1,1},{1,1},{1,1},{1,1},{1,1}}; //state of pins
int prevState[5][2] = {{1,1},{1,1},{1,1},{1,1},{1,1}}; //previous state of pins

int offset[5] = {0, 1, 2, 3, 4}; //THE OFFSET CANNOT BE ANY LARGER THAN ONTIME


int buttonPress(int i) {
    int change;
    currentState[i][0] = digitalRead(buttons[i][0]); //update the currentstate
    currentState[i][1] = digitalRead(buttons[i][1]);

    // compare the buttonState to its previous state
    //the change variable is +1 if button is pushed, then when released and pushed again we get a -1
    if ((currentState[i][0] == HIGH) && (prevState[i][0] == LOW)) {
    change = 1;
    } else if ((currentState[i][1] == HIGH) && (prevState[i][1] == LOW)) {
    change = -1;
    } else {
    change = 0;
    }
    prevState[i][0] = currentState[i][0];
    prevState[i][1] = currentState[i][1];
    return change;
}

void display7seg () { //this is the function that displays the numbers on the 2 digit 7 segments
    for (int k = 0; k < numDigits; k++) {
        if ((millis()-previousMillis-offset[k] >= onTime) && (alreadyUpdated[k] == 0)) { //offset fixes this isse
            alreadyUpdated[k] = 1;
            cycleState[k] = (cycleState[k] + 1) % 4; //our cycle state goes from 0 to 1 to 2 to 3 back to zero (we add one and take the remainder when dividing by 4)
            
            if (k == 0) {
                previousMillis = millis(); //kind of jank but it does the job
                for (int k = 0; k < numDigits; k++) {
                    alreadyUpdated[k]=0;
                }
            }
            //Serial.print("updating \t");
            //Serial.println(k);
            
            if (cycleState[k] == 0) {
                digitalWrite(twoDigitPins[k][0], LOW);
                digitalWrite(twoDigitPins[k][1], HIGH);

                digitalWrite(latch, LOW);
                shiftOut(data[k], clocks[k], MSBFIRST, Digits[counters[k] % 10]);
                digitalWrite(latch, HIGH);

            } else if (cycleState[k] == 1) {
                digitalWrite(latch, LOW);
                shiftOut(data[k], clocks[k], MSBFIRST, 0);
                digitalWrite(latch, HIGH);

            } else if (cycleState[k] == 2) {
                digitalWrite(twoDigitPins[k][0], HIGH);
                digitalWrite(twoDigitPins[k][1], LOW);

                digitalWrite(latch, LOW);
                shiftOut(data[k], clocks[k], MSBFIRST, Digits[counters[k] / 10]);
                digitalWrite(latch, HIGH);
            } else {
                digitalWrite(latch, LOW);
                shiftOut(data[k], clocks[k], MSBFIRST, 0);
                digitalWrite(latch, HIGH);
            }
        }
    }
    
}

void setup() { //normal void setup stuff

    pinMode(latch, OUTPUT); //latch and clock are outputs
    
    for (int i = 0; i < 5; i++) {
        pinMode(clocks[i], OUTPUT);
    }

    for (int i = 0; i < 5; i++) { //for all of the data pins
        pinMode(data[i], OUTPUT); //set them to outputs
    }

    for (int i = 0; i < 5; i++) { //for all of the 2 digit 7 segments
        for (int j = 0; j < 2; j++) {  //for each pin of each of them
            pinMode(twoDigitPins[i][j], OUTPUT); //set them to outputs
        }
    }

    for (int i = 0; i < 5; i++) { //for all of the 2 digit 7 segments
        for (int j = 0; j < 2; j++) {  //for each pin of each of them
            pinMode(buttons[i][j], INPUT_PULLUP); //set them to outputs
        }
    }

    Serial.begin(115200); //serial monitor if you want it  
}

void loop() {

    for (int i = 0; i < 5; i++) { //update the counter values
        counters[i] += buttonPress(i);
        if (counters[i] < 0) {
            counters[i]=0;
        }
        if (counters[i] > 99) {
            counters[i]=99;
        }
        //Serial.print(counters[i]);
        //Serial.print("\t");
    }
    //Serial.println();

    display7seg();

    //7731 runs in 10 seconds, hopefully this is fast enough...

}