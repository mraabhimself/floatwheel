#ifndef __KEY_H
#define __KEY_H

#include "hk32f030m.h"

#define POWER_BUTTON ((GPIOC->IDR >> 3) & 1)

typedef enum {
  NOT_PRESSED,
  SINGLE_PRESS,
  DOUBLE_PRESS,
  TRIPLE_PRESS = 4,
  LONG_PRESS = 3
} PowerButtonStates;

typedef enum {
  WAITING_FOR_PRESS,
  ON_PRESS,
  ON_RELEASE,
  ON_LONG_PRESS,
  PROCESS_PRESSES
} PowerButtonMonitorStates;

typedef enum { BUTTON_PRESSED, BUTTON_RELEASED } ButtonStates;

extern uint8_t powerButtonState;

void initializePowerButton(void);
void monitorPowerButton(void);

#endif
