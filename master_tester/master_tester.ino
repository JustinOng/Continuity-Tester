#include <LiquidCrystal.h>
#include "common/common.h"
#include <SoftwareSerial.h>

LiquidCrystal lcd(14, 15, 16, 17, 18, 19);
SoftwareSerial serial1(10, 11);

void setup() {
  resetPinStates();
  lcd.begin(16, 2);
  //use Hardware Serial for controlling the HC11
  Serial.begin(115200);
  serial1.begin(115200);
}

uint32_t last_ping_sent = 0;
uint32_t last_ping_received = 0;
uint8_t connected = 0;

uint8_t SerialWriteWaitAck(uint8_t data) {
  uint32_t start_time = millis();
  for(uint8_t i = 0; i < 3; i++) {
    Serial.write(data);
    start_time = millis();
    while(1) {
      if (Serial.available()) {
        last_ping_received = millis();
        if (Serial.read() == BIT_COMMAND) {
          return 1;
        }
      }
  
      if (millis() - start_time > PING_TIMEOUT) {
        break;
      }
    } 
  }
  return 0;
}

void checkForForeignConnections(void) {
  // i = local pin under test
  for(uint8_t i = 2; i <= 9; i++) {
    // j = foreign pin under test
    for(uint8_t j = 2; j <= 9; j++) {
      serial1.print("Testing ");
      serial1.print(i);
      serial1.print(" and ");
      serial1.println(j);
      
      resetPinsToInput();
      if (!SerialWriteWaitAck(BIT_COMMAND | BIT_RST)) {
        pin_states[j-2+8] |= BIT_ERROR;
        return;
      }
      
      if (!SerialWriteWaitAck(BIT_COMMAND | BIT_HIGH | (j-2))) {
        pin_states[j-2+8] |= BIT_ERROR;
        return;
      }
      
      uint8_t pin = i;

      for(uint8_t k = 2; k <= 9; k++) {
        // if !(k has a local connection and belongs to the same group as pin)
        // write LOW as well
        pinMode(k, OUTPUT);
        if (!(pin_states[k-2] & BIT_LOCAL && (pin_states[k-2] & 0x0F) == (pin_states[pin-2] & 0x0F))) {
          digitalWrite(k, LOW);
        }
      }

      pinMode(i, INPUT_PULLUP);

      // at this point, i is pulled up while j is also HIGH. If i is connected to j, i should read HIGH
      if (digitalRead(i) != HIGH) {
        /*serial1.print(i);
        serial1.print(" and ");
        serial1.print(j);
        serial1.println(" are not connected");*/

        //change to continue once it works
        continue;
      }
      
      if (!SerialWriteWaitAck(BIT_COMMAND | BIT_LOW)) {
        pin_states[j-2+8] |= BIT_ERROR;
        return;
      }

      // at this point, i is pulled up while j is LOW. If i is connected to j, i should read LOW
            
      if (digitalRead(i) == LOW) {
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

        serial1.print(i);
        serial1.print(" and ");
        serial1.print(j);
        serial1.println(" are connected");
      }
      else {
        /*serial1.print(i);
        serial1.println(" has not gone low");*/
      }
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
  
  if (connected) SerialWriteWaitAck(BIT_COMMAND | BIT_RST);
  
  checkForLocalConnections();
  resetPinsToInput();
  
  if (connected) {
    SerialWriteWaitAck(BIT_COMMAND | BIT_CHECKLOCAL);
    checkForForeignConnections();
  }

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
