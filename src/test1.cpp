#include <Arduino.h> //you don't need this if you're not using vscode
#include <RTClib.h> //rtc library
#include <SPI.h> //for i2c comms, needed for rtc library
//#include <TM1637Display.h> //4 digit 7 segment library, USING THE ONE WITH NO DELAYS

//Latch and clock are shared between all shift registers
int latch = 22; 

//I'm using arrays to define pins for the 2 digit LEDs, since it'll turn out to be very very helpful for our display7seg function

const int numDigits = 5;

int twoDigitPins[numDigits][2] = {{33, 34}, {35, 36}, {37, 38}, {39,40}, {41, 42}}; //This is an array inside an array!  I can explain this way more if you guys want

int data[numDigits] = {23, 24, 25, 27, 28}; //This is an array of the data pins for each of the modules
int clocks[numDigits] = {28, 29, 30, 31, 23};
int counters[numDigits] = {01, 02, 03, 04, 05};

int Digits[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 103}; //digits encoded in binary, for a common anode display?

//These variables are for the display7seg function
unsigned long onTime = 15; //how long each of the 2d segments is on and off for
int cycleTracker = 0; //for knowing what action we should be doing for displaying
int cycleState = 0; //similar to the previous variable
unsigned long previousMillis = 0; //unsigned longs are big numbers, which is good since they're gonna be storing large values


int buttons[5][2] = {{21,20}, {19,18}, {17,16}, {15,14}, {2,3}};
int change = 0;
int currentState[5][2] = {{1,1},{1,1},{1,1},{1,1},{1,1}}; //state of pins
int prevState[5][2] = {{1,1},{1,1},{1,1},{1,1},{1,1}}; //previous state of pins


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


//Upcoming function written by Clement, ask him if you have questions
void toggleColumns(bool rightOn) { //this function turns on one column of the 2 digit 7 segments and turns off the other
    if (!rightOn) {
        for (int i = 0; i < numDigits; i++) { //*change i < number of rows 
            digitalWrite(twoDigitPins[i][0], HIGH);
            digitalWrite(twoDigitPins[i][1], LOW);
        }
    } else {
        for (int i = 0; i < numDigits; i++) {
            digitalWrite(twoDigitPins[i][0], LOW);
            digitalWrite(twoDigitPins[i][1], HIGH);
        }
    }
}

//Upcoming function written by Kareem, ask him if you have questions
void display7seg () { //this is the function that displays the numbers on the 2 digit 7 segments
    if (millis()-previousMillis >= onTime) { //previousmillis keeps track of the last time we changed states, so we change states if it's been more than the onTime we set, so 3ms
        cycleState = (cycleState + 1) % 4; //our cycle state goes from 0 to 1 to 2 to 3 back to zero (we add one and take the remainder when dividing by 4)
        previousMillis = millis(); //we set previousmillis back to the current time in millisseconds
        Serial.println(cycleState);
        if (cycleState == 0) { //first state: display the ones digit of all 7 segments
            toggleColumns(true); //turn all the ones digits on and the tens digits off
    
            digitalWrite(latch, LOW); //turn latch low to update the data
            
            for (int i = 0; i < numDigits; i++) { //for all of the data pins
                shiftOut(data[i], clocks[i], MSBFIRST, Digits[counters[i] % 10]); //we write the 10s digit of whatever's in the counter array
                Serial.print("the tens digit being displayed is  \t ");
                Serial.println(counters[i] % 10);
            }
            digitalWrite(latch, HIGH);
            
        } else if (cycleState == 1) { //second state: display nothing on both displays 

            digitalWrite(latch, LOW);
             for (int i = 0; i < numDigits; i++) { //for all of the data pins
                shiftOut(data[i], clocks[i], MSBFIRST, 0); //we clear the displays
            }
            digitalWrite(latch, HIGH);

        } else if (cycleState == 2) {
            toggleColumns(false); //turn all the ones digits off and the tens digits on

            digitalWrite(latch, LOW);
            for (int i = 0; i < numDigits; i++) { //for all of the data pins
                shiftOut(data[i], clocks[i], MSBFIRST, Digits[counters[i] / 10]); //we write the ones digit of whatever's in the counter array
                Serial.print("the ones digit being displayed is  \t ");
                Serial.println(counters[i] / 10);
            }
            digitalWrite(latch, HIGH); //turn latch back on to update display
            
        } else {
            digitalWrite(latch, LOW);
            for (int i = 0; i < numDigits; i++) { 
                shiftOut(data[i], clocks[i], MSBFIRST, 0); 
            }
            digitalWrite(latch, HIGH);
        }
    }
}

void setup() { //normal void setup stuff

    pinMode(latch, OUTPUT); //latch and clock are outputs
    
    for (int i = 0; i < numDigits; i++) {
        pinMode(clocks[i], OUTPUT);
    }

    for (int i = 0; i < numDigits; i++) { //for all of the data pins
        pinMode(data[i], OUTPUT); //set them to outputs
    }

    for (int i = 0; i < numDigits; i++) { //for all of the 2 digit 7 segments
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
        Serial.print(counters[i]);
        Serial.print("\t");
    }
    Serial.println();

    display7seg();

    //7731 runs in 10 seconds, hopefully this is fast enough...

}