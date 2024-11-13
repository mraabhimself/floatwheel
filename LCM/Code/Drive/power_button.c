#include "power_button.h"

uint8_t powerButtonState = NOT_PRESSED;

void initializePowerButton(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void monitorPowerButton(void) {
  static uint8_t monitorState = WAITING_FOR_PRESS;
  static uint16_t debounceCounter = 0;
  static uint16_t longPressCount, pressCount = 0;

  switch (monitorState) {
  case WAITING_FOR_PRESS:
    if (POWER_BUTTON == BUTTON_PRESSED) {
      debounceCounter++;

      if (debounceCounter >= 20) {
        debounceCounter = 0;
        monitorState = ON_PRESS;
      }
    } else {
      debounceCounter = 0;
      longPressCount = 0;
    }
    break;

  case ON_PRESS:
    debounceCounter++;
    longPressCount++;

    if (POWER_BUTTON == BUTTON_RELEASED) {
      pressCount++;

      monitorState = ON_RELEASE;
      debounceCounter = 0;
    } else if (debounceCounter >= 1500) {
      debounceCounter = 0;

      monitorState = ON_LONG_PRESS;
      powerButtonState = LONG_PRESS;
    }

    break;

  case ON_RELEASE:
    longPressCount++;

    if (longPressCount <= 500) {
      if (POWER_BUTTON == BUTTON_PRESSED) {
        debounceCounter++;
        if (debounceCounter >= 2) {
          debounceCounter = 0;
          monitorState = ON_PRESS;
        }
      } else {
        debounceCounter = 0;
      }
    } else {
      longPressCount = 0;
      monitorState = PROCESS_PRESSES;
    }
    break;

  case ON_LONG_PRESS:
    if (POWER_BUTTON == BUTTON_RELEASED) {
      monitorState = WAITING_FOR_PRESS;
    }
    break;

  case PROCESS_PRESSES:
    switch (pressCount) {
    case 1:
      powerButtonState = SINGLE_PRESS;
      break;

    case 2:
      powerButtonState = DOUBLE_PRESS;
      break;

    case 3:
      powerButtonState = TRIPLE_PRESS;
      break;

    default:
      break;
    }

    pressCount = 0;
    monitorState = WAITING_FOR_PRESS;
    break;
  }
}
