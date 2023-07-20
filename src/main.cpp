#include <Arduino.h> //you don't need this if you're not using vscode
#include <RTClib.h> //rtc library
#include <SPI.h> //for i2c comms, needed for rtc library
#include <TM1637Display.h> //4 digit 7 segment library, USING THE ONE WITH NO DELAYS

int latch = 22; 

const int numDigits = 5;

int twoDigitPins[5][2] = {{33, 34}, {35, 36}, {38, 37}, {39,40}, {41, 42}}; //This is an array inside an array!  I can explain this way more if you guys want

int data[5] = {23, 24, 25, 26, 27}; //This is an array of the data pins for each of the modules
int clocks[5] = {28, 29, 30, 31, 32};
int counters[5] = {01, 02, 03, 04, 05};

int Digits[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 103}; //digits encoded in binary, for a common anode display?

unsigned long onTime = 5; //how long each of the 2d segments is on and off for
int cycleState[5] = {0, 0, 0, 0, 0}; //similar to the previous variable
int alreadyUpdated[5] = {0, 0, 0, 0, 0};
unsigned long previousMillis = 0; //unsigned longs are big numbers, which is good since they're gonna be storing large values

int buttons[5][2] = {{19,18}, {17,16}, {15,14}, {2,3}, {4,5}};
int change = 0;
int currentState[5][2] = {{1,1},{1,1},{1,1},{1,1},{1,1}}; //state of pins
int prevState[5][2] = {{1,1},{1,1},{1,1},{1,1},{1,1}}; //previous state of pins

int offset[5] = {0, 1, 2, 3, 4}; //THE OFFSET CANNOT BE ANY LARGER THAN ONTIME

// EVERYTHING ABOVE THIS IS FOR 7 SEGMENTS

// EVERYTHING BELOW THIS IS FOR RTC/TIMERS/TXACA

RTC_DS3231 rtc; //our rtc

bool arrivedYet = false; //the patient hasn't arrived

DateTime now; //we'll use this to get real time data
uint32_t timeSinceArrival = 0; //time since arrival tracker, unsigned longs since it might be a big number
uint32_t timeSinceActivate = 0; //time since activate tracker

//our 4 digit 7 segments
TM1637Display clockDisplay = TM1637Display(49, 48); //first pin is clock, second pin is digital input/output
TM1637Display arrivalDisplay = TM1637Display(45, 44);
TM1637Display activateDisplay = TM1637Display(47, 46);

const int InPinArrival1 = 9;
const int InPinArrival2 = 8;

int CurrentStateArrival1 = HIGH;
int PreviousStateArrival1 = HIGH;

long arrivalHoursPassed = 0;
long arrivalMinutesPassed = 0;
long arrivalSecondsPassed = 0;

bool StopwatchDisplayArrival1 = false;

const int InPinActivate1 = 7;
const int InPinActivate2 = 6;

int CurrentStateActivate1 = HIGH;
int PreviousStateActivate1 = HIGH;

long activateHoursPassed = 0;
long activateMinutesPassed = 0;
long activateSecondsPassed = 0;

bool StopwatchDisplayActivate1 = false;

int txaButton = 10;
int caButton = 11;
int txaRed = 52;
int txaGreen = 53;
int caRed = 50;
int caGreen = 51;

unsigned long previousMillisRTCNow = 0;

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

void startRTC() { //starts up the RTC
    if (!rtc.begin()) {
        Serial.println("No RTC found, waiting");
        while (1) delay(10);
    }
    if (rtc.now().year() == 2000) {//if the oscillator isn't running yet
        Serial.println("RTC not running yet, setting the time");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //all this syntax here just takes the date and time found at the time of compilation
    } 
}

//Upcoming function written by Clement, ask him if you have questions
void displayCurrentTime() {
    int displaytime = (now.hour() * 100) + now.minute();
	clockDisplay.showNumberDecEx(displaytime, 0b11100000, true); //first argument is the number, second argument just sets the colon position, third argument tells leading zeros
}

//Upcoming function written by Kareem, ask him if you have questions
void displayArrivalTime() {
    // button check stuff
    CurrentStateArrival1 = digitalRead (InPinArrival1);
    if ((CurrentStateArrival1 == LOW) && (PreviousStateArrival1 == HIGH))
    {
        if (StopwatchDisplayArrival1 == false){
            timeSinceArrival = now.unixtime();
            StopwatchDisplayArrival1 = true;
        }
    }
    PreviousStateArrival1 = CurrentStateArrival1;

    if (digitalRead(InPinArrival2) == LOW) {
        int displaytime = (arrivalHoursPassed * 100) + arrivalMinutesPassed;
        arrivalDisplay.showNumberDecEx(displaytime, 0b11100000, true);
        StopwatchDisplayArrival1 = false;
    }

    //stopwatch stuff

    if (StopwatchDisplayArrival1 == true)
    {
        arrivalHoursPassed = ( long(now.unixtime() - timeSinceArrival) / 3600 )% 100; //gets the hours passed (converts from unixtime to hours)
        arrivalMinutesPassed = ( long(now.unixtime() - timeSinceArrival)% 3600 ) / 60;//gets the minutes of the time (converted from unixtime)
        arrivalSecondsPassed = long( now.unixtime() - timeSinceArrival);
        int displaytime = (arrivalHoursPassed * 100) + arrivalMinutesPassed;
        if (arrivalSecondsPassed % 2 == 0) {
            arrivalDisplay.showNumberDecEx(displaytime, 0b11100000, true);
        } else {
            arrivalDisplay.showNumberDecEx(displaytime, 0b00000000, true);
        }
    }
}

void displayActivateTime() { 
    // button check stuff
    CurrentStateActivate1 = digitalRead (InPinActivate1);
    
    if ((CurrentStateActivate1 == LOW) && (PreviousStateActivate1 == HIGH))
    {
        if (StopwatchDisplayActivate1 == false){
            timeSinceActivate = now.unixtime();
            StopwatchDisplayActivate1 = true;
        }

    }
    PreviousStateActivate1 = CurrentStateActivate1;

    if (digitalRead(InPinActivate2) == LOW) {
        int displaytime = (activateHoursPassed * 100) + activateMinutesPassed;
        activateDisplay.showNumberDecEx(displaytime, 0b11100000, true);
        StopwatchDisplayActivate1 = false;
    }

    //stopwatch stuff

    if (StopwatchDisplayActivate1 == true)
    {
        activateHoursPassed = ( long(now.unixtime() - timeSinceActivate) / 3600 )% 100; //gets the hours passed (converts from unixtime to hours)
        activateMinutesPassed = ( long(now.unixtime() - timeSinceActivate)% 3600 ) / 60;//gets the minutes of the time (converted from unixtime)
        activateSecondsPassed = long( now.unixtime() - timeSinceActivate);
        int displaytime = (activateHoursPassed * 100) + activateMinutesPassed;
        if (activateSecondsPassed % 2 == 0) {
            activateDisplay.showNumberDecEx(displaytime, 0b11100000, true);
        } else {
            activateDisplay.showNumberDecEx(displaytime, 0b00000000, true);
        }
    }
}

void updateTXACA() {

    if (digitalRead (txaButton) == LOW) {
        digitalWrite(txaRed, LOW);
        digitalWrite(txaGreen, HIGH);
    }

    if (digitalRead (caButton) == LOW) {
        digitalWrite(caRed, LOW);
        digitalWrite(caGreen, HIGH);
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
    
    //EVERYTHING ABOVE THIS IS FOR 7 SEGMENTS
    //EVERYTHING BELOW THIS IS RTC/TIMERS/TXACA STUFF

    pinMode(txaButton, INPUT_PULLUP);
    pinMode(caButton, INPUT_PULLUP);

    pinMode(txaRed, OUTPUT);
    pinMode(txaGreen, OUTPUT);
    pinMode(caRed, OUTPUT);
    pinMode(caGreen, OUTPUT);

    pinMode (InPinArrival1, INPUT_PULLUP); //more buttons
    pinMode (InPinActivate1, INPUT_PULLUP);

    pinMode (InPinArrival2, INPUT_PULLUP); //more buttons
    pinMode (InPinActivate2, INPUT_PULLUP);

    digitalWrite(txaRed, HIGH); 
    digitalWrite(caRed, HIGH); 
    digitalWrite(txaGreen, LOW); 
    digitalWrite(caGreen, LOW); 

    Serial.begin(115200); //serial monitor if you want it  

    startRTC(); //starts the RTC, check out the function above

    clockDisplay.setBrightness(5); //you can set the brightness of the 4 digit 7 segments
    clockDisplay.clear(); //clear the display

    arrivalDisplay.setBrightness(5); //you can set the brightness of the 4 digit 7 segments
    arrivalDisplay.clear(); //clear the display

    activateDisplay.setBrightness(5); //you can set the brightness of the 4 digit 7 segments
    activateDisplay.clear(); //clear the display


}

void loop() {
    if (millis() - previousMillisRTCNow >= 500) {
        now = rtc.now(); //note to Kareem you probably don't want to call this so frequently but for now its fine
        previousMillisRTCNow = millis();
    }
    
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

    if (clockDisplay.update()) {
        displayCurrentTime(); //we wrote this function for you
    }

    display7seg();

    if (arrivalDisplay.update()) {
        displayArrivalTime();
    }

    display7seg();

    if (activateDisplay.update()) {
        displayActivateTime();
    }
    updateTXACA(); //update the txa and ca things

}
