#include <Arduino.h> //you don't need this if you're not using vscode
#include <RTClib.h> //rtc library
#include <SPI.h> //for i2c comms, needed for rtc library
#include <TM1637Display.h> //4 digit 7 segment library, USING THE ONE WITH NO DELAYS

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

int CurrentStateArrival2 = HIGH;
int PreviousStateArrival2 = HIGH;

bool StopwatchDisplayArrival1 = false;

const int InPinActivate1 = 7;
const int InPinActivate2 = 6;

int CurrentStateActivate1 = HIGH;
int PreviousStateActivate1 = HIGH;

int CurrentStateActivate2 = HIGH;
int PreviousStateActivate2 = HIGH;

bool StopwatchDisplayActivate1 = false;

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
        StopwatchDisplayArrival1 = false;
    }

    //stopwatch stuff

    if (StopwatchDisplayArrival1 == true)
    {
        long hoursPassed = ( long(now.unixtime() - timeSinceArrival) / 3600 )% 100; //gets the hours passed (converts from unixtime to hours)
        long minutesPassed = ( long(now.unixtime() - timeSinceArrival)% 3600 ) / 60;//gets the minutes of the time (converted from unixtime)
        long secondsPassed = long( now.unixtime() - timeSinceArrival);
        int displaytime = (hoursPassed * 100) + secondsPassed;
        if (secondsPassed % 2 == 0) {
            arrivalDisplay.showNumberDecEx(displaytime, 0b11100000, true);
        } else {
            arrivalDisplay.showNumberDecEx(displaytime, 0b00000000, true);
        }
    }
}

void displayActivateTime() { 
    // button check stuff
    CurrentStateActivate1 = digitalRead (InPinActivate1);
    CurrentStateActivate2 = digitalRead (InPinActivate2);
    
    if ((CurrentStateActivate1 == LOW) && (PreviousStateActivate1 == HIGH))
    {
        if (StopwatchDisplayActivate1 == false){
            timeSinceActivate = now.unixtime();
            StopwatchDisplayActivate1 = true;
        }

    }
    PreviousStateActivate1 = CurrentStateActivate1;
    PreviousStateActivate2 = CurrentStateActivate2;

    if ((CurrentStateActivate2 == LOW) && (PreviousStateActivate2 == HIGH)) {
        long hoursPassed = ( long(now.unixtime() - timeSinceActivate) / 3600 )% 100; //gets the hours passed (converts from unixtime to hours)
        long minutesPassed = ( long(now.unixtime() - timeSinceActivate)% 3600 ) / 60;//gets the minutes of the time (converted from unixtime)
        long secondsPassed = long( now.unixtime() - timeSinceActivate);
        int displaytime = (hoursPassed * 100) + secondsPassed;
        activateDisplay.showNumberDecEx(displaytime, 0b11100000, true);
        
        StopwatchDisplayActivate1 = false;
    }

    //stopwatch stuff

    if (StopwatchDisplayActivate1 == true)
    {
        long hoursPassed = ( long(now.unixtime() - timeSinceActivate) / 3600 )% 100; //gets the hours passed (converts from unixtime to hours)
        long minutesPassed = ( long(now.unixtime() - timeSinceActivate)% 3600 ) / 60;//gets the minutes of the time (converted from unixtime)
        long secondsPassed = long( now.unixtime() - timeSinceActivate);
        int displaytime = (hoursPassed * 100) + secondsPassed;
        if (secondsPassed % 2 == 0) {
            activateDisplay.showNumberDecEx(displaytime, 0b11100000, true);
        } else {
            activateDisplay.showNumberDecEx(displaytime, 0b00000000, true);
        }
    }
}

void setup() { //normal void setup stuff

    pinMode (InPinArrival1, INPUT_PULLUP); //more buttons
    pinMode (InPinActivate1, INPUT_PULLUP);

    pinMode (InPinArrival2, INPUT_PULLUP); //more buttons
    pinMode (InPinActivate2, INPUT_PULLUP);

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
    
    now = rtc.now(); //note to Kareem you probably don't want to call this so frequently but for now its fine
    
    if (clockDisplay.update()) {
        displayCurrentTime(); //we wrote this function for you
    }
    if (arrivalDisplay.update()) {
        displayArrivalTime();
    }
    if (activateDisplay.update()) {
        displayActivateTime();
    }


    Serial.print(digitalRead(6) + "\t");
    Serial.print(digitalRead(7)+ "\t");
    Serial.print(digitalRead(8) + "\t");
    Serial.print(digitalRead(9) + "\n");
}
