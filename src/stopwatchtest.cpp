#include <Arduino.h> //you don't need this if you're not using vscode
#include <RTClib.h> //rtc library
#include <SPI.h> //for i2c comms, needed for rtc library
#include <TM1637Display.h> //4 digit 7 segment library

RTC_DS3231 rtc; //our rtc

bool arrivedYet = false; //the patient hasn't arrived

DateTime now; //we'll use this to get real time data
uint32_t timeSinceArrival = 0; //time since arrival tracker, unsigned longs since it might be a big number
uint32_t timeSinceActivate = 0; //time since activate tracker

//our 4 digit 7 segments
TM1637Display clockDisplay = TM1637Display(3, 4); //first pin is clock, second pin is digital input/output
TM1637Display arrivalDisplay = TM1637Display(12, 13);



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
    long secondsPassed = long( now.unixtime() - timeSinceActivate);
    Serial.println(now.unixtime());
    Serial.println(hoursPassed);
    Serial.println(minutesPassed);
    Serial.println(secondsPassed);
    int displaytime = (hoursPassed * 100) + minutesPassed;
    arrivalDisplay.showNumberDecEx(displaytime, 0b11100000, true);

}


void setup() { //normal void setup stuff

  Serial.begin(115200); //serial monitor if you want it  

  startRTC(); //starts the RTC, check out the function above

  clockDisplay.setBrightness(5); //you can set the brightness of the 4 digit 7 segments
  clockDisplay.clear(); //clear the display
  
  arrivalDisplay.setBrightness(5); //you can set the brightness of the 4 digit 7 segments
  arrivalDisplay.clear(); //clear the display
}

void loop() {
    
    now = rtc.now(); //note to Kareem you probably don't want to call this so frequently but for now its fine
    displayCurrentTime(); //we wrote this function for you
    displayArrivalTime();
    delay(1000);

}
    


  

