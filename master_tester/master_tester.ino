#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include "common/common.h"

LiquidCrystal lcd(14, 15, 16, 17, 18, 19);
SoftwareSerial serial(10, 11);

void setup() {
  resetPinStates();
  lcd.begin(16, 2);
  //use Hardware Serial for controlling the HC11 at 9600
  Serial.begin(9600);
  //use Software Serial on 10/11 tx/rx for debugging
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
