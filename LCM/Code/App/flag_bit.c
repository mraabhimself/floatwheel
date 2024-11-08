#include "flag_bit.h"
#include "datatypes.h"


uint8_t Power_Flag = PWR_FLAG_INITIAL;
uint8_t Charge_Flag = CHG_FLAG_INITIAL;
uint8_t Lightbar_Display_Flag = LIGHTBAR_MODE_INITIAL;
uint8_t Footpad_Flag = FOOTPAD_FLAG_INITIAL;
uint8_t Buzzer_Flag = BUZZER_FLAG_OFF;

/*
	Power_Display_Flag = 0; // Off
	Power_Display_Flag = 1; // 4.08V ~ 4.2V 10 bars
	Power_Display_Flag = 2; // 4.05V ~ 4.08V 9 bars
	Power_Display_Flag = 3; // 3.96V ~ 4.05V 8 bars
	Power_Display_Flag = 4; // 3.87V ~ 3.96V 7 bars
	Power_Display_Flag = 5; // 3.78V ~ 3.87V 6 bars
	Power_Display_Flag = 6; // 3.70V ~ 3.78V 5 bars
	Power_Display_Flag = 7; // 3.62V ~ 3.70V 4 bars
	Power_Display_Flag = 8; // 3.50V ~ 3.62V 3 bars
	Power_Display_Flag = 9; // 3.35V ~ 3.50V 2 bars
	Power_Display_Flag = 10; // 2.80V ~ 3.35V 1 bar
*/
uint8_t Power_Display_Flag = 0;

/*
	Usart_Flag = Vesc_Data_Ready
*/
uint8_t Vesc_Data_Ready = 0;

/*
    Buzzer delay time
*/
uint16_t Buzzer_Time = 0;

/*
    Charging time
*/
uint16_t Charge_Time = 0;
/*
    Flashlight duration
*/
uint16_t Flashlight_Time = 0;
/*
    Power delay time
*/
uint16_t Power_Time = 0;
/*
USART communication time
*/
uint16_t Usart_Time = 0;
/*
	ADC sampling time.
*/
uint16_t ADC_Time = 0;
/*
	Time the board has been idle (aka not running)
*/
uint16_t Idle_Time = 0;
/*
	Shutdown counter, milliseconds and minutes
*/
uint16_t Shutdown_Time_S = 0;
uint8_t Shutdown_Time_M = 0;
/*
	ADC1
*/
float ADC1_Val = 0;
/*
	ADC2
*/
float ADC2_Val = 0;
/*
	Charging Port Voltage
*/
float Charge_Voltage = 0;
/*
	Light profile
*/
uint8_t Light_Profile = LIGHT_PROFILE_INITIAL;
/*
	WS2812 Status bar brightness
*/
uint8_t Status_Bar_Brightness = 0;
/*
	 Buzzer frequency in BPM, beats per minute, ranging from 60 to 180, corresponding to a duty cycle of 70 to 100)
*/
uint8_t Buzzer_Frequency = 0;
/*
	 When the button is double-clicked without pressing it, the headlight stays on for 3 seconds.
*/
uint16_t Flashlight_Detection_Time = 0;
/*
	Charging current and voltage original values
*/
float Charge_Current = 0;
/*
	V_I = 0: Sampling charging current.
	V_I = 1: Sampling charging voltage.
*/
uint8_t V_I = 1;
/*
	Counter used while detecting end of charging
*/
uint16_t Shutdown_Cnt = 0;
/*
	Charger detection time.
*/
uint16_t Charger_Detection_1ms = 0;
/*
	The current brightness of the headlights, to slowly ramp up to target brightness
*/
int Current_Headlight_Brightness = 0;
int Target_Headlight_Brightness = 0;
