#include <Arduino.h>

#define BIT_LOCAL 0x80
#define BIT_ERROR 0x40
#define BIT_LEADER 0x20
#define BIT_HASCON 0x08

extern uint8_t pin_states[8];

char getPinState(uint8_t pin);

void checkForLocalConnections(void);

void checkForForeignConnections(void);

void resetPinStates(void);