#include <LiquidCrystal.h>
//#include <SoftwareSerial.h>
#include "common/common.h"

LiquidCrystal lcd(14, 15, 16, 17, 18, 19);
//SoftwareSerial serial(10, 11);

void setup() {
  resetPinStates();
  lcd.begin(16, 2);
  //use Hardware Serial for controlling the HC11 at 9600
  Serial.begin(9600);
  //use Software Serial on 10/11 tx/rx for debugging
}

uint32_t last_ping_sent = 0;
uint32_t last_ping_received = 0;
uint8_t connected = 0;

void checkForForeignConnections(void) {
  for(uint8_t i = 2; i <= 9; i++) {
    for(uint8_t j = 2; j <= 9; j++) {
      Serial.write(BIT_COMMAND | BIT_HIGH | j);
      while(1) {
        if (Serial.available()) {
          if (Serial.read() == BIT_COMMAND) break;
        }
      }
      uint8_t leader = 0;
      
      if (pin_states[i-2] & BIT_LOCAL) {
        leader = pin_states[i-2] & 0x0F;
      }
      
      // set all pins that dont belong to the same group as i to LOW to provide a common ground reference
      for(uint8_t k = 2; k <= 9; k++) {
        // if i does not belong to a group OR k belongs to the same group as i, set it low
        if (!(pin_states[i-2] & BIT_LOCAL) || pin_states[k-2] & 0x0F == leader) {
          pinMode(k, INPUT_PULLUP);
        }
      }
      
      if (getGroupState(i) == HIGH) {
        uint8_t leader;
        //if i already has local connection, use the i's leader instead of i when setting leader of this new foreign group
        if (pin_states[i-2] & BIT_LOCAL) {
          leader = pin_states[i-2] & 0x0F;
        }
        else {
          leader = (i-2);
        }
        
        pin_states[i-2] |= BIT_FOREIGN;
        pin_states[i-2] = (pin_states[i-2] & 0xF0) | leader;

        pin_states[j-2+8] |= BIT_FOREIGN;
        pin_states[j-2+8] = (pin_states[j-2] & 0xF0) | leader;
      }
    }
  }
}

void SerialWriteWaitAck(uint8_t data) {
  Serial.write(data);
  while(1) {
    if (Serial.available()) {
      last_ping_received = millis();
      if (Serial.read() == BIT_COMMAND) break;
    }
  }
}

void loop() {
  uint32_t current_time = millis();
  if (current_time - last_ping_sent > PING_INTERVAL_MS) {
    Serial.write(0x00);
    last_ping_sent = current_time;
  }

  while(Serial.available()) {
    last_ping_received = current_time;
    connected = true;
    
    switch(Serial.read()) {
      
    }
  }

  if (current_time - last_ping_received > PING_TIMEOUT) {
    connected = false;
  }
  
  resetPinStates();

  //SerialWriteWaitAck(BIT_COMMAND | BIT_RST);
  
  checkForLocalConnections();
  resetPinsToInput();
  
  //SerialWriteWaitAck(BIT_COMMAND | BIT_CHECKLOCAL);

  for(uint8_t i = 0; i < 16; i++) {
    lcd.setCursor(i*2 % 16, i > 7);
    if (pin_states[i] & BIT_ERROR) {
      lcd.print("E ");
    }
    else if (pin_states[i] & (BIT_LOCAL | BIT_FOREIGN)) {
      // has local connection
      // print id of group leader
      lcd.print(pin_states[i] & 0x0F);
      lcd.print(' ');
    }
    else {
      lcd.print("F ");
    }
  }

  lcd.setCursor(15, 1);
  if (!connected) {
    lcd.print("N");
  }
  else {
    lcd.print("Y");
  }
}
