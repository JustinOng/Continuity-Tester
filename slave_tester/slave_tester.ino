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

  while(1) {
    for(uint8_t i = 2; i <= 9; i++) {
      pinMode(i, OUTPUT);
      digitalWrite(i, HIGH);
    }
  }
}

uint32_t last_ping_sent = 0;
uint32_t last_ping_received = 0;
uint8_t connected = 0;

void loop() {
  uint32_t current_time = millis();
  uint8_t pin;
  if (current_time - last_ping_sent > PING_INTERVAL_MS) {
    Serial.write(0x00);
    last_ping_sent = current_time;
  }

  while(Serial.available()) {
    last_ping_received = current_time;
    connected = true;

    uint8_t data = Serial.read();
    
    switch(data & 0x70) {
      case BIT_HIGH:
        pin = data & 0x0F;
        digitalWrite(pin, HIGH);

        for(uint8_t k = 2; k <= 9; k++) {
          // if k has a local connection and belongs to the same group as pin
          // write HIGH as well
          if (pin_states[k-2] & BIT_LOCAL && (pin_states[k-2] & 0x07) + 2 == pin) {
            digitalWrite(k, HIGH);
          }
          else {
            digitalWrite(k, LOW);
          }
        }
        
        // acknowledge command
        Serial.write(BIT_COMMAND);
        break;
      case BIT_RST:
        resetPinStates();
        Serial.write(BIT_COMMAND);
        break;
      case BIT_CHECKLOCAL:
        checkForLocalConnections();
        Serial.write(BIT_COMMAND);
        break;
    }
  }

  if (current_time - last_ping_received > PING_TIMEOUT) {
    connected = false;
  }

  for(uint8_t i = 0; i < 8; i++) {
    lcd.setCursor(i*2, 0);
    if (pin_states[i] & BIT_ERROR) {
      lcd.print("E ");
    }
    else if (pin_states[i] & BIT_LOCAL) {
      // has local connection
      // print id of group leader
          
      lcd.print(pin_states[i] & 0x07);
      lcd.print(' ');
    }
    else {
      lcd.print("F ");
    }
  }
  
  lcd.setCursor(0, 1);
  if (!connected) {
    lcd.print("N");
  }
  else {
    lcd.print("Y");
  }
}
