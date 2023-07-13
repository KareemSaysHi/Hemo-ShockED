#include <Arduino.h>

// pin declarations
const int buttons[5][2] = {{28.29}, {30,31}, {32,33}, {34,35}, {36,37}};
int change = 0 ;
int display1 = 0;
int display2 = 0;

int currentState[5][2] = {{1,1},{1,1},{1,1},{1,1},{1,1}}; //state of pins
int prevState[5][2] = {{1,1},{1,1},{1,1},{1,1},{1,1}}; //previous state of pins

void setup() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 2; j++) {
            pinMode(buttons[i][j], INPUT_PULLUP);
        }
    }
    Serial.begin(115200);
}

void loop() {
  change = buttonPress(0);
  display1 = display1 + change;
  change = buttonPress(1);
  display2 = display2 + change;
  Serial.print(display1);
  Serial.print("\t");
  Serial.println(display2);
  delay(100);
}

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