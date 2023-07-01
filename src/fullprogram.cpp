#include <Arduino.h> //you don't need this if you're not using vscode
#include <RTClib.h> //rtc library
#include <SPI.h> //for i2c comms, needed for rtc library
#include <TM1637Display.h> //4 digit 7 segment library

RTC_DS3231 rtc; //our rtc

bool arrivedYet = 0; //the patient hasn't arrived

DateTime now; //we'll use this to get real time data
unsigned long timeSinceArrival = 0; //time since arrival tracker, unsigned longs since it might be a big number
unsigned long timeSinceActivate = 0; //time since activate tracker

//our 4 digit 7 segments
TM1637Display clockDisplay = TM1637Display(22, 23); //first pin is clock, second pin is digital input/output
TM1637Display arrivalDisplay = TM1637Display(24, 25);
TM1637Display activateDisplay = TM1637Display(26, 27);

//Latch and clock are shared between all shift registers
int latch = 12; 
int clock = 13;

//I'm using arrays to define pins for the 2 digit LEDs, since it'll turn out to be very very helpful for our display7seg function
int twoDigitPins[2][2] = {{6, 7}, {8, 9}}; //This is an array inside an array!  I can explain this way more if you guys want
int data[2] = {2, 3}; //This is an array of the data pins for each of the modules
int counters[2] = {0, 0};

int Digits[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 103}; //digits encoded in binary, for a common anode display?

//These variables are for the display7seg function
unsigned long onTime = 3; //how long each of the 2d segments is on and off for
int cycleTracker = 0; //for knowing what action we should be doing for displaying
int cycleState = 0; //similar to the previous variable
unsigned long previousMillis = 0; //unsigned longs are big numbers, which is good since they're gonna be storing large values

//Upcoming function written by Clement, ask him if you have questions
void toggleColumns(bool rightOn) { //this function turns on one column of the 2 digit 7 segments and turns off the other
    if (!rightOn) {
        for (int i = 0; i < 2; i++) { //*change i < number of rows 
            digitalWrite(twoDigitPins[i][0], HIGH);
            digitalWrite(twoDigitPins[i][1], LOW);
        }
    } else {
        for (int i = 0; i < 2; i++) {
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
        
        if (cycleState == 0) { //first state: display the ones digit of all 7 segments
            toggleColumns(true); //turn all the ones digits on and the tens digits off
    
            digitalWrite(latch, LOW); //turn latch low to update the data

            for (int i = 0; i < 2; i++) { //for all of the data pins
                shiftOut(data[i], clock, MSBFIRST, Digits[counters[i] % 10]); //we write the 10s digit of whatever's in the counter array
            }
            
            digitalWrite(latch, HIGH); //turn latch back on to update display
            
        } else if (cycleState == 1) { //second state: display nothing on both displays 

            digitalWrite(latch, LOW);
             for (int i = 0; i < 2; i++) { //for all of the data pins
                shiftOut(data[i], clock, MSBFIRST, 0); //we clear the displays
            }
            digitalWrite(latch, HIGH);

        } else if (cycleState == 2) {
            toggleColumns(false); //turn all the ones digits off and the tens digits on

            digitalWrite(latch, LOW);
            for (int i = 0; i < 2; i++) { //for all of the data pins
                shiftOut(data[i], clock, MSBFIRST, Digits[counters[i] / 10]); //we write the ones digit of whatever's in the counter array
            }
            digitalWrite(latch, HIGH);
            
        } else {
            digitalWrite(latch, LOW);
            for (int i = 0; i < 2; i++) { 
                shiftOut(data[i], clock, MSBFIRST, 0); 
            }
            digitalWrite(latch, HIGH);
        }
    }
}

//Upcoming function written by Kareem, ask him if you have questions
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
    if (!arrivedYet) {
        timeSinceActivate = now.unixtime(); //start a stopwatch, stores the time in unixtime (look it up to get more info)
        arrivedYet = true;
    }
    long hoursPassed = ( long(now.unixtime() - timeSinceActivate) / 3600 )% 100; //gets the hours passed (converts from unixtime to hours)
    long minutesPassed = ( long(now.unixtime() - timeSinceActivate)% 3600 ) / 60;//gets the minutes of the time (converted from unixtime)
    /*
    for debugging long secondsPassed = long( now.unixtime() - timeSinceActivate);
    Serial.println(now.unixtime());
    Serial.println(hoursPassed);
    Serial.println(minutesPassed);
    Serial.println(secondsPassed);
    int displaytime = (hoursPassed * 100) + minutesPassed; 
    */
    arrivalDisplay.showNumberDecEx(displaytime, 0b11100000, true);

}

void displayActivateTime() {
    Serial.println("you gotta work for it");
    //this one's very similar to the arrival time one, you guys gotta figure it out, plus you gotta learn unixtime :)
}

void setup() { //normal void setup stuff
    
  pinMode(latch, OUTPUT); //latch and clock are outputs
  pinMode(clock, OUTPUT);
  
  for (int i = 0; i < 2; i++) { //for all of the data pins
    pinMode(data[i], OUTPUT); //set them to outputs
  }

  for (int i = 0; i < 2; i++) { //for all of the 2 digit 7 segments
    for (int j = 0; j < 2; i++) {  //for each pin of each of them
        pinMode(twoDigitPins[i][j], OUTPUT); //set them to outputs
    }
  }

  Serial.begin(115200); //serial monitor if you want it  

  startRTC(); //starts the RTC, check out the function above

  clockDisplay.setBrightness(5); //you can set the brightness of the 4 digit 7 segments
  clockDisplay.clear(); //clear the display
}

void loop() {
    
    //Ok the rest is up to you guys

    // STEP 1: UPDATE TIME AND CLOCK LEDS
    
    now = rtc.now(); //note to Kareem you probably don't want to call this so frequently but for now its fine

    displayCurrentTime(); //we wrote this function for you
    Serial.println("you gotta work for it"); //IF ARRIVAL BUTTON PUSHED: displayArrivalTime() We wrote this function for you
    Serial.println("you gotta work for it"); //IF ACTIVATE BUTtON PUSHED: displayActivateTime() You gotta write this one yourself

    //things to think about: do you want the displays to be on if you haven't started the timers yet?  You can control the brightness of these displays, I did one in void setup

    // STEP 2: UPDATE THE COUNTERS USING BUTTON PRESSES
    
    //to update the displays, edit the values in the counters array I defined at the top
    Serial.println("you gotta work for it"); // this is all you we are giving you none of this :)

    // STEP 3: UPDATE THE 2 DIGIT LEDS
    display7seg(); //I made this very easy, thats it :D

}
    


  

