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

uint32_t last_ping_sent = 0;
uint32_t last_ping_received = 0;
uint8_t connected = 0;

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
  checkForLocalConnections();

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
