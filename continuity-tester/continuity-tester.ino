#include <LiquidCrystal.h>

#define BIT_LOCAL 0x80
#define BIT_ERROR 0x40
#define BIT_LEADER 0x20
#define BIT_HASCON 0x08

LiquidCrystal lcd(14, 15, 16, 17, 18, 19);

//pin states:
//0bLEG0HXXX
//L = local connection
//E = has error, unexpected state(shorted to something it shoudnt be?)
//G = is group leader
//H = has connection, used to indicate that XXX refers to a valid pin number
//    (XXX cannot be set on the group leader because the group might consist of more than 2 pins)
//XXX = 0-7 of pin index connected to

uint8_t pin_states[8] = {};

char getPinState(uint8_t pin) {
  //returns 0 for LOW, 1 for HIGH and -1 for floating
  //stateWhenPulledDown refers to value read when no pullup is enabled
  pinMode(pin, INPUT);
  uint8_t stateWhenPulledDown = digitalRead(pin);
  pinMode(pin, INPUT_PULLUP);
  uint8_t stateWhenPulledUp = digitalRead(pin);
  
  if (stateWhenPulledDown == stateWhenPulledUp) {
    //state is same meaning external HIGH/LOW is applied to the pin
    return stateWhenPulledDown;
  }
  else {
    //state different meaning it followed the pullup/down
    return -1;
  }
}

void checkForLocalConnections(void) {
  //i: pin under test
  //j: pin that is pulled high
  //k: all other pins, floating (input)

  //the first pin in the group (two or more pins connected) is the "leader", will have BIT_LOCAL and BIT_LEADER set. BIT_HASCON will not be set.
  //any other pins in the same group will have BIT_LOCAL and BIT_HASCON set, with XXX set to the index of the "leader"
  for(uint8_t i = 2; i <= 9; i++) {
    //if pin has not already been tested and found to have a local short
    if (pin_states[i-2] & BIT_LOCAL) {
      Serial.print("Pin ");
      Serial.print(i-2);
      Serial.println(" already has a short, skipping");
      continue;
    }
    Serial.print("Outer pin ");
    Serial.println(i-2);

    //iterate over all pins other than the one under test, setting one to pulled high and the rest as inputs
    //if pinState(i) == HIGH, then i and the pulled high pins are shorted
    for(uint8_t j = 2; j <= 9; j++) {
      //if pin is same as i or pin has a local short, continue
      if (j == i || pin_states[j-2] & BIT_LOCAL) continue;
      
      Serial.print("Inner pin ");
      Serial.println(j-2);

      for(uint8_t k = 2; k <= 9; k++) {
        //sets all other pins to floating
        if (k == i || k == j) continue;

        pinMode(k, INPUT);
      }
      
      //sets j to HIGH
      pinMode(j, OUTPUT);
      digitalWrite(j, HIGH);

      //test i
      if (getPinState(i) == 1) {
        //i and j are shorted
        Serial.print("Pin ");
        Serial.print(i-2);
        Serial.println(" is group leader");
        pin_states[i-2] |= (BIT_LOCAL | BIT_LEADER);

        Serial.print("Pin ");
        Serial.print(j-2);
        Serial.print(" is in group ");
        Serial.println(i-2);
        pin_states[j-2] |= (BIT_LOCAL | BIT_HASCON);
        //set XXX to the index of i
        pin_states[j-2] = (pin_states[j-2] & 0xF8) | (i-2);
      }
    }
  }
}

void resetPinStates(void) {  
  for(uint8_t i = 2; i <= 9; i++) {
    pinMode(i, INPUT);
    pin_states[i-2] = 0x00;
  }
}

void setup() {
  resetPinStates();

  lcd.begin(16, 2);
  Serial.begin(115200);
}

void loop() {
  resetPinStates();
  checkForLocalConnections();

  for(uint8_t i = 0; i < 8; i++) {
    lcd.setCursor(i*2, 0);
    if (pin_states[i] & BIT_ERROR) {
      lcd.print("E ");
    }
    else if (pin_states[i] & BIT_LOCAL) {
      //has local connection
      
      //if BIT_HASCON is not set, then this pin is the group leader, display just the pin number
      if (pin_states[i] & BIT_LEADER) {
        lcd.print(i);
        lcd.print(' ');
      }
      else if (pin_states[i] & BIT_HASCON) {
        //pin is part of a group and is not the leader
        //print the leader pin number (XXX)
        lcd.print(pin_states[i] & 0x07);
        lcd.print(' ');
      }
      else {
        lcd.print("??");
      }
    }
    else {
      lcd.print("F ");
    }
  }
  delay(100);
}
