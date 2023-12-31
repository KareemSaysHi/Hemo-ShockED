#include <Arduino.h>
#include <RTClib.h>
#include <SPI.h>

RTC_DS3231 rtc;

int i = 2;

void setup() {
    Serial.begin(115200);
    if (!rtc.begin()) {
        Serial.println("No RTC found, waiting");
        while (1) delay(10);
    }
    
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //might need F?

}

void loop() {
    DateTime now = rtc.now(); //DateTime::TIMESTAMP_DATE gives some constant from the DateTime class that we called

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    delay(1000);
}