void display7seg2 (int number) {
    cycleTracker = millis() % (2*onTime + 2*offTime);
    
    if ((cycleTracker < onTime)) {
      if (cycleState == 3) {
        cycleState = 0;

        digitalWrite(digit1, LOW);
        digitalWrite(digit2, HIGH);

        digitalWrite(latch, LOW);
        shiftOut(data, clock, MSBFIRST, Digits[number % 10]);
        digitalWrite(latch, HIGH);
      }
    } else if ((cycleTracker < (onTime + offTime))) {
       if (cycleState == 0) {
        cycleState = 1;

        digitalWrite(latch, LOW);
        shiftOut(data, clock, MSBFIRST, 0);
        digitalWrite(latch, HIGH);

        //digitalWrite(digit1, LOW);
        //digitalWrite(digit2, LOW);
      }
    } else if (cycleTracker < (onTime + offTime + onTime)) {
      if (cycleState == 1) {
        cycleState = 2;

        digitalWrite(digit1, HIGH);
        digitalWrite(digit2, LOW);

        digitalWrite(latch, LOW);
        shiftOut(data, clock, MSBFIRST, Digits[int(number / 10)]);
        digitalWrite(latch, HIGH);
      }
    } else if (cycleTracker < (onTime + offTime + onTime + offTime)) {
      if (cycleState == 2) {
        cycleState = 3;

        digitalWrite(latch, LOW);
        shiftOut(data, clock, MSBFIRST, 0);
        digitalWrite(latch, HIGH);

        //digitalWrite(digit1, LOW);
        //digitalWrite(digit2, LOW);
      }
    }     
}