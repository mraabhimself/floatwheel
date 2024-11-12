#ifndef LCM_H
#define LCM_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool floatPackageSupported;
  float avgInputCurrent;
  int dutyCycleNow; // duty when running
  int pitch;        // pitch when idle
  float inpVoltage;
  long rpm;
  char state;
  char fault;
  bool isForward;
  bool isHandtest;
  bool isOldPackage;
} dataPackage;

typedef struct {
  uint8_t headlightBrightness;
  uint8_t headlightIdleBrightness;
  uint8_t statusbarBrightness;
  // StatusBarIdleMode statusBarIdleMode;
  uint8_t dutyBeep;
  // float chargeCutoffVoltage;
  // uint8_t autoShutdownTime;
  bool boardOff;
  // BootAnimation bootAnimation;
  bool isSet;
  bool debug;
} lcmConfig_t;

typedef enum {
  LCM_PACKET_START = 2,
  LCM_PACKET_END = 3,
  FLOAT_PACKAGE_ID = 101,
  LCM_CHARGESTATE_START = 151
} LcmPacketFlags;

typedef enum {
  COMMAND_LCM_POLL = 24,
  COMMAND_LCM_CTRL = 25,
  COMMAND_LCM_INFO = 26,
  COMMAND_LCM_CHARGESTATE = 28,
  COMMAND_LCM_DEBUG = 99
} LcmCommands;

typedef enum {
  STARTUP = 0,
  RUNNING = 1,
  RUNNING_TILTBACK = 2,
  RUNNING_WHEELSLIP = 3,
  RUNNING_UPSIDEDOWN = 4,
  RUNNING_FLYWHEEL = 5,  // we remain in "RUNNING" state in flywheel mode,
                         // but then report "RUNNING_FLYWHEEL" in rt data
  FAULT_ANGLE_PITCH = 6, // skipped 5 for compatibility
  FAULT_ANGLE_ROLL = 7,
  FAULT_SWITCH_HALF = 8,
  FAULT_SWITCH_FULL = 9,
  FAULT_DUTY = 10, // unused but kept for compatibility
  FAULT_STARTUP = 11,
  FAULT_REVERSE = 12,
  FAULT_QUICKSTOP = 13,
  DISABLED = 15
} FloatState;

typedef enum {
  // Core light control
  HEADLIGHT_BRIGHTNESS = 0,
  HEADLIGHT_IDLE_BRIGHTNESS = 1,
  STATUSBAR_BRIGHTNESS = 2,
  // Additional light config
  STATUS_BAR_IDLE_MODE = 10,
  BOOT_ANIMATION = 11,
  // Misc config
  DUTY_BEEP = 50,
  // Sys commands
  POWER_OFF = 100,
  CHARGE_CUTOFF = 101,
  AUTO_SHUTDOWN = 102,
  FACTORY_RESET = 200,
  DEBUG = 255,
} LcmControlCommands;

typedef enum {
  BOOT_DEFAULT = 0,
  BOOT_REDWHITEBLUE = 1,
  BOOT_RAINBOW = 2,
} BootAnimation;

typedef enum {
  IDLE_MODE_HYBRID = 0,
  IDLE_MODE_BATTERY = 1,
  IDLE_MODE_KNIGHT_RIDER = 2,
} StatusBarIdleMode;

/// LCM Flag Types
typedef enum {
  PWR_FLAG_INITIAL,  // Just powered on.
  PWR_FLAG_BOOTING,  // VESC booting up.
  PWR_FLAG_BOOTED,   // VESC boot-up completed.
  PWR_FLAG_VESC_OFF, // VESC powered off, the board is powered by the charger.
  PWR_FLAG_START_POWEROFF, // Start the shutdown sequence of VESC
} Power_Flags;

typedef enum {
  CHG_FLAG_INITIAL,     // Just powered on
  CHG_FLAG_READY,       // Ready to charge
  CHG_FLAG_IN_PROGRESS, // Charging in progress
  CHG_FLAG_COMPLETE,    // Battery charged
} Charge_Flags;

typedef enum {
  LIGHTBAR_MODE_INITIAL,    // Just powered on.
  LIGHTBAR_MODE_BATTERY,    // Display battery level.
  LIGHTBAR_MODE_NO_BATTERY, // Do not display battery level.
  LIGHTBAR_MODE_SHUTDOWN,   // Show Shutdown animation
} Lightbar_Modes;

typedef enum {
  FOOTPAD_FLAG_INITIAL, // Just powered on.
  FOOTPAD_FLAG_LEFT,    // Left 5 LEDs blue, Right 5 LEDs off (adc1 > 2.5V, adc2
                        // < 2.5V).
  FOOTPAD_FLAG_RIGHT,   // Left 5 LEDs off, Right 5 LEDs blue (adc1 < 2.5V, adc2
                        // > 2.5V).
  FOOTPAD_FLAG_BOTH,    // All 10 LEDs blue (adc1 > 2.5V, adc2 > 2.5V).
  FOOTPAD_FLAG_RIDING,  // All 10 LEDs off.
  FOOTPAD_FLAG_FLYWHEEL, // Flywheel mode, pattern
} Footpad_Flags;

typedef enum {
  BUZZER_FLAG_OFF,    // Buzzer_Flag = 0; Off
  BUZZER_FLAG_SINGLE, // Buzzer_Flag = 1; Single beep
  BUZZER_FLAG_DOUBLE, // Buzzer_Flag = 2; Double beep
} Buzzer_Flags;

typedef enum {
  LIGHT_PROFILE_INITIAL, // Just powered on.
  LIGHT_PROFILE_1, // Headlight bightness low, Lightpad brightness high, beep
                   // once
  LIGHT_PROFILE_2, // Headlight bightness mid, Lightpad brightness mid, beep
                   // twice
  LIGHT_PROFILE_3, // Headlight bightness high, Lightpad brightness low, beep
                   // thrice
} Light_Profiles;

#endif