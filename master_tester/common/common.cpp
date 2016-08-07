#include "common.h"

extern uint8_t pin_states[16] = {};

//pin states:
//0bLEG0HXXX
//L = local connection
//E = has error, unexpected state(shorted to something it shoudnt be?)
//G = is group leader
//H = has connection, used to indicate that XXX refers to a valid pin number
//    (XXX cannot be set on the group leader because the group might consist of more than 2 pins)
//XXX = 0-7 of pin index connected to

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

char getGroupState(uint8_t pin) {
  if (pin_states[pin-2] & BIT_LOCAL) {
    uint8_t leader = pin_states[pin-2] & 0x0F;
    
    for(uint8_t i = 2; i <= 9; i++) {
      if (pin_states[i-2] & 0x0F == leader) {
        pinMode(i, INPUT);
      }
    }
    
    uint8_t stateWhenPulledDown = digitalRead(pin);
    
    for(uint8_t i = 2; i <= 9; i++) {
      if (pin_states[i-2] & 0x0F == leader) {
        pinMode(i, INPUT_PULLUP);
      }
    }
    
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
  else {
    return getPinState(pin);
  }
}

void checkForLocalConnections(void) {
  //i: pin under test
  //j: pin that is pulled high
  //k: all other pins, floating (input)

  //the first pin in the group (two or more pins connected) is the "leader", will have BIT_LOCAL and BIT_LEADER set
  //any other pins in the same group will have BIT_LOCAL and BIT_HASCON set, with XXX set to the index of the "leader"
  for(uint8_t i = 2; i <= 9; i++) {
    //if pin has not already been tested and found to have a local short
    if (pin_states[i-2] & BIT_LOCAL) {
      continue;
    }

    //iterate over all pins other than the one under test, setting one to pulled high and the rest as inputs
    //if pinState(i) == HIGH, then i and the pulled high pins are shorted
    for(uint8_t j = 2; j <= 9; j++) {
      //if pin is same as i or pin has a local short, continue
      if (j == i || pin_states[j-2] & BIT_LOCAL) continue;

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
        pin_states[i-2] |= (BIT_LOCAL | BIT_LEADER | (i-2));
        pin_states[j-2] |= BIT_LOCAL;
        //set XXX to the index of i
        pin_states[j-2] = (pin_states[j-2] & 0xF0) | (i-2);
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

void resetPinsToInput(void) {  
  for(uint8_t i = 2; i <= 9; i++) {
    pinMode(i, INPUT);
  }
}