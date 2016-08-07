#include <Arduino.h>

#define PING_INTERVAL_MS 100
#define PING_TIMEOUT 500

#define BIT_LOCAL 0x80
#define BIT_FOREIGN 0x40
#define BIT_ERROR 0x20
#define BIT_LEADER 0x10

#define BIT_COMMAND 0x80

#define BIT_HIGH 0x40
// BIT_HIGH triggers a mode where the pin number provided and the group that the pin belongs to (if any)
// are all HIGH, with all remaining pins LOW.

#define BIT_RST 0x20
// BIT_HIZ resets all states ie set all as input

#define BIT_CHECKLOCAL 0x10

extern uint8_t pin_states[16];
// pin_states store the state of each pin, with the first 8 belonging to master
// and next 8 belonging to the slave

char getPinState(uint8_t pin);
char getGroupState(uint8_t pin);

void checkForLocalConnections(void);

void checkForForeignConnections(void);

void resetPinStates(void);

void resetPinsToInput(void);