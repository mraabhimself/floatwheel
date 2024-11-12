#include "task.h"
#include "flag_bit.h"
#include "math.h"
#include "eeprom.h"
#include "datatypes.h"
#include "key.h"
#include "led.h"
#include "vesc_uasrt.h"
#include "ws2812.h"
#include "lcm.h"

#define  BOOT_ANIMATION_COUNT  3
#define  STATUS_BAR_IDLE_MODE_COUNT  2

/**************************************************
+ * Reset LCM Config
+ **************************************************/
static void lcmConfigReset(void)
{
	lcmConfig.isSet = false;
	lcmConfig.debug = false;
	lcmConfig.headlightBrightness = 0;
	lcmConfig.headlightIdleBrightness = 0;
	lcmConfig.statusbarBrightness = 5;
	lcmConfig.boardOff = 0;
	lcmConfig.dutyBeep = DUTY_CYCLE_BEEP_PERCENT;
	/*
	lcmConfig.statusBarIdleMode = DEFAULT_IDLE_MODE;
	lcmConfig.chargeCutoffVoltage = 0;
	lcmConfig.bootAnimation = BOOT_DEFAULT;
	lcmConfig.autoShutdownTime = AUTO_SHUTDOWN_TIME;
	
	EEPROM_ReadByte(BOOT_ANIMATION, &lcmConfig.bootAnimation);
	if (lcmConfig.bootAnimation < 0 || lcmConfig.bootAnimation >= BOOT_ANIMATION_COUNT) {
		lcmConfig.bootAnimation = BOOT_DEFAULT;
	}

	EEPROM_ReadByte(DUTY_BEEP, &lcmConfig.dutyBeep);
	if (lcmConfig.dutyBeep < 1 || lcmConfig.dutyBeep > 100) {
		lcmConfig.dutyBeep = DUTY_CYCLE_BEEP_PERCENT;
	}

	EEPROM_ReadByte(STATUS_BAR_IDLE_MODE, &lcmConfig.statusBarIdleMode);
	if (lcmConfig.statusBarIdleMode < 0 || lcmConfig.statusBarIdleMode >= STATUS_BAR_IDLE_MODE_COUNT) {
		lcmConfig.statusBarIdleMode = DEFAULT_IDLE_MODE;
	}

	EEPROM_ReadByte(AUTO_SHUTDOWN, &lcmConfig.autoShutdownTime);
	if (lcmConfig.autoShutdownTime <= 0) {
		lcmConfig.autoShutdownTime = AUTO_SHUTDOWN_TIME;
	} */
}

// brightnesses for Gear 1, 2, 3:
int headlight_brightnesses[] = { 0, 150, 255 };
int status_brightnesses[] = { WS2812_1_BRIGHTNESS, WS2812_2_BRIGHTNESS, WS2812_3_BRIGHTNESS };

/**************************************************
 * @brie   :KEY1_Task()
 **************************************************/
void KEY1_Task(void)
{
	if(KEY1_State == 0)// || Power_Flag == PWR_FLAG_VESC_OFF)  //充电器供电按键不起作用
	{
		return;
	}
	
	switch(KEY1_State)
	{
		case 1:         // Click
			if(Power_Flag != PWR_FLAG_BOOTED)
			{
				Power_Flag = PWR_FLAG_BOOTING;  // VESC power on
				lcmConfigReset();
			}
		break;

		case 2:         // Double click
			if(Power_Flag == PWR_FLAG_BOOTED) // Power on completed
			{
				lcmConfig.isSet = false; // Ignore LCM config when manually changing brightness
				Light_Profile++;
				if (Light_Profile > LIGHT_PROFILE_3)
				{
					Light_Profile = LIGHT_PROFILE_1;
				}
			}
		break;

		case 3:         // Long press
			if (Power_Flag < PWR_FLAG_VESC_OFF) {
				Power_Flag = PWR_FLAG_START_POWEROFF;  // VESC power off
				Power_Time = 0;
			}
		break;

		case 4:         // Three presses
			if(Power_Flag == PWR_FLAG_BOOTED) // Boot completed
			{
				Idle_Time = 0;
				if(Buzzer_Flag == BUZZER_FLAG_DOUBLE)
				{
					Buzzer_Flag = BUZZER_FLAG_SINGLE;
				}
				else
				{
					Buzzer_Flag = BUZZER_FLAG_DOUBLE;
				}
			}
		break;
	}

	KEY1_State = 0;

	// Reset back to showing battery percentage for a couple seconds
}

/**************************************************
 * @brie   :WS2812_Power_Display()
 * @note   :display 1..10 leds depending on power level
 **************************************************/
static void WS2812_Power_Display(uint8_t brightness)
{
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	// 20% and below: red
	// 40% and below: yellow
	// > 40% white
	r = brightness;
	if (Battery_Level > 2)
		g = brightness;
	if (Battery_Level > 4)
		b = brightness;
	
	if (Battery_Level > 0) {
		WS2812_Set_AllColours(1, Battery_Level, r, g, b);
	} else {
		// Two purple LEDs in the center, only needed for dev/debug (happens right after boot)
		//WS2812_Set_AllColours(5, 6, brightness, 0, brightness);
	}
	WS2812_Refresh();
}

/**************************************************
 * @brie   : WS2812_VESC()
 * @note   : Display VESC status
 **************************************************/
static void WS2812_VESC(void)
{
	uint8_t i;
	uint8_t pos, red;
	uint8_t green = 0;
	uint8_t blue = Status_Bar_Brightness;
	if (data.rpm > 250) {
		unsigned int fade = 1 + (data.rpm - 250) / 25;	// values from 1 to 31 (max rpm allowed is 1000)
		blue = blue / fade;
	}
	if (data.floatPackageSupported) {
		// make footpad indicators teal if float package commands are received successfully!
		blue = blue / 2;
		green = blue;
	}
	
	switch(Footpad_Flag)
	{
		case FOOTPAD_FLAG_LEFT:// Half Foot Sensors: adc1>2.5V  adc2<2.5V
				WS2812_Set_AllColours(1, 5,0,green, blue);
		break;
		
		case FOOTPAD_FLAG_RIGHT:// Half Foot Sensors: adc1<2.5V  adc2>2.5V
				WS2812_Set_AllColours(6, 10,0,green, blue);
		break;
		
		case FOOTPAD_FLAG_BOTH:// Both Foot Sensors: adc1>2.5V  adc2>2.5V
				WS2812_Set_AllColours(1, 10,0,green,blue);
		break;
			
		case FOOTPAD_FLAG_RIDING:// Riding
			
			if (Battery_Level < 3) {
				// Voltage below 30%?
				// Display 1/2 red dots at full brightness above anything else
				WS2812_Power_Display(Status_Bar_Brightness);
			}
			else if (data.dutyCycleNow > 90) {
				WS2812_Set_AllColours(1, NUM_LEDS,255,0,0);
			}
			else if (data.dutyCycleNow > 85) {
				WS2812_Set_AllColours(1, NUM_LEDS-1,255,0,0);
			}
			else if (data.dutyCycleNow > 80) {
				WS2812_Set_AllColours(1, NUM_LEDS-2,Status_Bar_Brightness,Status_Bar_Brightness/2,0);
			}
			else if (data.dutyCycleNow > 70) {
				WS2812_Set_AllColours(1, NUM_LEDS-3,Status_Bar_Brightness/3,Status_Bar_Brightness/3,0);
			}
			else if (Battery_Level < 4) {
				// Voltage below 40%?
				// Display 1/2/3 red dots at full brightness
				WS2812_Power_Display(Status_Bar_Brightness);
			}
			else {
				WS2812_Set_AllColours(1, NUM_LEDS,0,0,0);
			}
		break;

		case FOOTPAD_FLAG_FLYWHEEL:
			// Flywheel Mode: just a rando pattern fpr now
			red = Power_Time % 255;
			green = (Power_Time + 100) % 255;
		  	blue = (Power_Time - 100) % 255;
		  	pos = (Power_Time/100) % 10;
			WS2812_Set_Colour(pos,red,green,blue);
		break;			
		default:
		break;
	}
	WS2812_Refresh();
}


/**************************************************
 * @brie   :WS2812_Boot()
 * @note   :Boot Animation
 **************************************************/
void WS2812_Boot(void)
{
	uint8_t i;
	uint8_t num = floor(Power_Time / (VESC_BOOT_TIME/10)) + 1;
	uint8_t bootAnims[10][3] = {
		// Default (blue...green)
		//{{10,0,30}, {9,3,27}, {8,6,24}, {7,9,21}, {6,12,18}, {5,15,15}, {4,18,12}, {3,21,9}, {2,24,6}, {1,27,3}},
		{25,0,30}, {22,2,27}, {18,4,24}, {15,6,21}, {12,8,18}, {8,10,15}, {6,12,12}, {4,14,9}, {2,16,6}, {0,18,6},
		// Rainbow
		//{{30,0,0}, {30,15,0}, {30,30,0}, {15,30,0}, {0,30,0}, {0,30,15}, {0,30,30}, {0,15,30}, {0,0,30}, {15,0,30}},
		// Red White Blue
		//{{30,0,0}, {30,30,30}, {0,0,30}, {30,0,0}, {30,30,30}, {0,0,30}, {30,0,0}, {30,30,30}, {0,0,30}, {30,0,0}}
};

	/*if (lcmConfig.bootAnimation < 0 || lcmConfig.bootAnimation >= BOOT_ANIMATION_COUNT) {
		// Invalid boot animation
		lcmConfig.bootAnimation = BOOT_DEFAULT;
	}*/

	while (num > 10) {
		num -= 10;
	}
	for (i=0;i<num;i++) {
		WS2812_Set_Colour(i,bootAnims[i][0],bootAnims[i][1],bootAnims[i][2]);
	}

	for (i = num; i < 10; i++) {
		WS2812_Set_Colour(i,0,0,0);
	}

	WS2812_Refresh();
}

void WS2812_Shutdown(void)
{
	uint8_t brightness = 100;
	int num = 10 - floor(Power_Time / 100);
	if (num < 1) {
		num = 1;
	}
	WS2812_Set_AllColours(num, num, brightness / (11 - num), 0, 0);
	WS2812_Refresh();
}

uint8_t status_brightness = 1;
/**************************************************
 * @brie   : WS2812_Calc_Bri()
 * @note   : Pulsate brightness
 * @param  : times 1 means 200ms
**************************************************/
static uint8_t WS2812_Calc_Bri(uint8_t cnt)
{	
	if(cnt < 50)
	{
		status_brightness++;
	}
	else
	{
		status_brightness--;
	}
	
	if(status_brightness < 1)
	{
		status_brightness = 1;
	}
	
	if(status_brightness > 50)
	{
		status_brightness = 50;
	}
	
	return status_brightness;
}

/**************************************************
 * @brie   :WS2812_Charge()
 * @note   :Power LED display while charging
 **************************************************/
static void WS2812_Charge(void)
{
	static uint8_t cnt = 0;
	WS2812_Power_Display(WS2812_Calc_Bri(cnt));
	cnt++;
	if(cnt == 100)
	{
		cnt = 0;
	}
	
	WS2812_Refresh();
}	

static void WS2812_Disabled(void)
{
	int brightness = Status_Bar_Brightness;
	if (brightness < 20)
		brightness = 20;

	// 2 red LEDs in the center
	WS2812_Set_AllColours(5, 6, brightness, 0, 0);
	WS2812_Refresh();
}

static void WS2818_Knight_Rider(uint8_t brightness) {
	#define ANIMATION_TICK_TIME 6
	#define TAIL_LENGTH 4
	static uint8_t frame = 0;
	static int8_t direction = 1;
	static int8_t position = 0;

	if (frame >= ANIMATION_TICK_TIME) {
		frame = 0;

		for (uint8_t i = 0; i < NUM_LEDS; i++) {
			int8_t distanceToTail = (direction == 1) ? position - i : i - position;

			// Allow wrap around effect
			if (direction == 1 && i > position) {
				distanceToTail = i + position;
			} else if (direction == -1 && i < position) {
				distanceToTail = (NUM_LEDS - position - 1) + (NUM_LEDS - i - 1);
			}


			uint8_t brightness = (distanceToTail >= 0 && distanceToTail <= TAIL_LENGTH) ?
								Status_Bar_Brightness - distanceToTail * (Status_Bar_Brightness / TAIL_LENGTH) : 0;

			
			WS2812_Set_Colour(i,brightness,0,0);
		}

		// Move the LED position
		position += direction;

		// Change direction when reaching the ends
		if (position == NUM_LEDS - 1  || position == 0) {
			direction *= -1;
		}
		WS2812_Refresh();
	}

	frame++;
}

// Idle animation:
static void WS2812_Idle()
{
	if (Idle_Time > 3000) {
		if (Battery_Level == 1) {
			// Voltage <= 10%? Flash bright red!
			WS2812_Set_AllColours(1, 10, 255, 20, 20);
			WS2812_Refresh();
			if (Idle_Time > 3040) {
				Idle_Time = 0;
			}
		}
		else {
			if (data.isOldPackage)
				WS2812_Set_AllColours(1, 10, 255, 20, 255);
			else
				WS2818_Knight_Rider(Status_Bar_Brightness);
		}
		return;
	}
	// Battery mode
	WS2812_Power_Display(Status_Bar_Brightness);
}

static void WS2812_Handtest(void)
{
	static int pulsate = 0;
	int brightness = Status_Bar_Brightness;
	if (brightness < 20)
		brightness = 20;
	pulsate++;
	if (pulsate > 50)
		pulsate = 0;

	// 4 LEDs in the center
	WS2812_Set_AllColours(4, 7, brightness, pulsate, 0);
	if(ADC1_Val > 2.0)
		WS2812_Set_Colour(0, 0, 0, brightness);
	if(ADC2_Val > 2.0)
		WS2812_Set_Colour(9, 0, 0, brightness);

	WS2812_Refresh();
}

/**************************************************
 * @brie   :WS2812_Task()
 * @note   :WS2812任务 
 * @param  :无
 * @retval :无
 **************************************************/
void WS2812_Task(void)
{
	uint8_t i;
	
	if(Charge_Flag == CHG_FLAG_COMPLETE) // Battery fully charged
	{
		WS2812_Set_AllColours(1,10,50,150,50);	// white with a strong green tint
		WS2812_Refresh();
		return;
	}
	if(Charge_Flag == CHG_FLAG_IN_PROGRESS) // Charge display pattern (pulsating led)
	{
		WS2812_Charge();
		return;
	}

	if (Lightbar_Display_Flag == LIGHTBAR_MODE_SHUTDOWN) {
		WS2812_Shutdown();
		return;
	}

	if(Power_Flag == PWR_FLAG_INITIAL || (Power_Flag == PWR_FLAG_VESC_OFF && Charge_Flag == CHG_FLAG_INITIAL))
	{
		// Board is off
		WS2812_Set_AllColours(1,10,0,0,0);
		WS2812_Refresh();
		Lightbar_Display_Flag = LIGHTBAR_MODE_INITIAL;
		Footpad_Flag = FOOTPAD_FLAG_INITIAL;
		Battery_Level = 0;
		return;
	}

	if(Power_Flag == PWR_FLAG_BOOTING)
	{
		Idle_Time = 0;
		WS2812_Boot();
		return;
	}
	
	if (Power_Flag > PWR_FLAG_BOOTED) {
		WS2812_Refresh();
		Idle_Time = 0;
		return;
	}
	
	// Power Flag must be 2, aka board is ready or running
	if (lcmConfig.isSet) {
		Status_Bar_Brightness = lcmConfig.statusbarBrightness;
	}
	else if (Light_Profile >= LIGHT_PROFILE_1 && Light_Profile <= LIGHT_PROFILE_3)
	{
		Status_Bar_Brightness = status_brightnesses[Light_Profile - 1];
	}

	if (data.state == DISABLED) {
		WS2812_Disabled();
	}
	else if (data.isHandtest) {
		WS2812_Handtest();
	}
	else {
		if (Lightbar_Display_Flag == LIGHTBAR_MODE_BATTERY) {
			// Idle state - no footpads pressed
			WS2812_Idle();	// Idle animation
		} else {
			Idle_Time = 0;
			WS2812_VESC();
		}
	}
}

/**************************************************
 * @brie   :Power_Task()
 * @note   :电源任务 
 * @param  :无
 * @retval :无
 **************************************************/
void Power_Task(void)
{
	static uint8_t power_flag_last = 0; //上一次的状态
	static uint8_t power_step = 0;

	if (Power_Flag == 4) {
		if(Power_Time > VESC_SHUTDOWN_TIME)
		{
			Power_Flag = PWR_FLAG_VESC_OFF;
		}
	}
	
	if(power_flag_last == Power_Flag && Power_Flag != PWR_FLAG_BOOTING)
	{
		return;
	}
	power_flag_last = Power_Flag;
	
	switch(Power_Flag)
	{
		case PWR_FLAG_BOOTING://VESC Power On
			PWR_ON;
			switch(power_step)
			{
				case 0:
					Power_Time = 0;
					power_step = 1;
				break;
				
				case 1:
					if(Power_Time > VESC_BOOT_TIME)
					{
						Power_Flag = PWR_FLAG_BOOTED; // Boot completed
						Light_Profile = LIGHT_PROFILE_1; // The default setting is 1st gear after power-on.
						Buzzer_Flag = BUZZER_FLAG_DOUBLE;    // The default buzzer sounds when powering on
						power_step = 0;
						Lightbar_Display_Flag = LIGHTBAR_MODE_BATTERY;
					}
				break;
			}
			
		break;	

		case PWR_FLAG_VESC_OFF:// VESC is shut down (either auto-shutdown or button press)
			Lightbar_Display_Flag = LIGHTBAR_MODE_INITIAL;
			PWR_OFF 
		break;

		case PWR_FLAG_START_POWEROFF:// New Power state for shutdown sequence
			Lightbar_Display_Flag = LIGHTBAR_MODE_SHUTDOWN;
		default:
		break;
	}
}

void CheckPowerLevel(float battery_voltage)
{
	float battVoltages[10] = {
		3.0,
		3.38,
		3.485,
		3.571,
		3.651,
		3.74,
		3.827,
		3.908,
		4.01,
		4.054
	}; //P42A


	// Default: Between zero and min voltage
	Battery_Level = 0;
	int i = 0;
	while (i < 9 && battery_voltage > battVoltages[i] ) {
		i++;
	}

	Battery_Level = i+1;
}

/**************************************************
 * @brie   :Charge_Task()
 * @note   :Check for charge start/end conditions
 **************************************************/
#ifdef ADV
void Charge_Task(void)
{
	static uint8_t charge_step = 0;
	bool isAboveCutoff = false;//lcmConfig.chargeCutoffVoltage > 0 && Charge_Voltage > lcmConfig.chargeCutoffVoltage;

	if(Charge_Flag > CHG_FLAG_INITIAL)
	{
		if(V_I == 0 && Charge_Time > 150)
		{
			if((Charge_Current < CHARGE_CURRENT && Charge_Current > 0) || isAboveCutoff) {
				Shutdown_Cnt++;
				if(Shutdown_Cnt>2000)
				{
					if (isAboveCutoff || Charge_Voltage > FULL_VOLTAGE) {
						Charge_Flag = CHG_FLAG_COMPLETE;
					}
					else {
						Charge_Flag = CHG_FLAG_INITIAL;
					}
					Charge_Time = 0;
					V_I = 1;
					LED1_ON; // Use ADC3 to measure charge voltage
					CHARGE_OFF;
				}
			}
			else
			{
				Shutdown_Cnt = 0;
			}
		}
		else
		{
			if((Charge_Flag == CHG_FLAG_IN_PROGRESS) && (Charge_Time > 150))
			{
        CheckPowerLevel((Charge_Voltage + 1) / BATTERY_PACK_SERIES);
      }
			if((Charge_Flag == CHG_FLAG_COMPLETE) && (Shutdown_Cnt > 10))
			{
				if (Charge_Voltage < CHARGING_VOLTAGE)
				{
					// wait for charger to get unplugged to reset back to normal state
					Charge_Flag = CHG_FLAG_INITIAL;
					charge_step = 0;
					Charge_Voltage = 0;
					Charger_Detection_1ms = 0;
				}
			}
		}
	}
	else //	Charge_Flag == CHG_FLAG_INITIAL
	{
		charge_step = 0;
		if(Charge_Voltage > CHARGING_VOLTAGE)// && (Charge_Current > 0.1))
		{
			if(Charger_Detection_1ms > CHARGER_DETECTION_DELAY)
			{
				if (Charge_Flag != CHG_FLAG_IN_PROGRESS)
					Charge_Flag = CHG_FLAG_READY;
				Lightbar_Display_Flag = LIGHTBAR_MODE_INITIAL;
			}
		}
		else {
			Charger_Detection_1ms = 0;
			return;
		}
	}

	switch(charge_step)
	{
		case 0:
			Charge_Time = 0;
			charge_step = 1;
		break;
		
		case 1:
			if(Charge_Time > 1000)  //��ʱ1S
			{
				charge_step = 2;
			}
		break;
		
		case 2:
			CHARGE_ON;
			Charge_Flag = CHG_FLAG_IN_PROGRESS;
			charge_step = 3;
			//Power_Flag = PWR_FLAG_BOOTING;	// Boot the VESC
		break;
		
		case 3:
			Charge_Time = 0;
			charge_step = 4;
		break;
			
		case 4:	
			if(Charge_Time > DETECTION_SWITCH_TIME) 
			{
				V_I = 1;
				Charge_Time = 0;
				LED1_ON; // Use ADC3 to measure charge voltage
				charge_step = 5;
			}
		break;
			
		case 5:
			if(Charge_Time > DETECTION_SWITCH_TIME) 
			{
				V_I = 0;
				Charge_Time = 0;
				LED1_OFF; // Use ADC3 to measure charge current
				charge_step = 4;
			}		
		break;
			
		default:
		break;
		
	}
}
#endif

/**************************************************
 * @brief  :Set_Headlights_Brightness()
 * @note   :
 * Brightness can be positive (forward) or negative (backward) - 0..255 (off to max bright)
 * Map to 9999[MIN] ... 0[MAX]
 */
static void Set_Headlights_Brightness(int brightness)
{
	if (brightness > 0) { // FORWARD
		LED_F_OFF;
		LED_B_ON;
	}
	else if (brightness == 0) {
		if (Target_Headlight_Brightness == 0) {
			LED_B_OFF;
			LED_F_OFF;
		}
	}
	else { // BACKWARD
		LED_B_OFF;
		LED_F_ON;
		brightness = -brightness;
	}
	int mapped_brightness = 9999 - brightness * 40;
	if (mapped_brightness < 0)
		mapped_brightness = 0;
	TIM_SetCompare2(TIM1, mapped_brightness);
}

/**************************************************
 * @brief  :Headlights_Task()
 * @note   :Handle direction changes and fading of headlights
 * Smooth fading during transitions is achieved by using positive and 
 * negative brightnesses for forward/backward
 **************************************************/
void Headlights_Task(void)
{
	static uint8_t light_profile_last = 0;
	static bool isForward = false;

	if (Flashlight_Time < 10) {
		return;
	}
	Flashlight_Time = 0;

	if(Power_Flag != PWR_FLAG_BOOTED) // Lights off 
	{
		LED_B_OFF;
		LED_F_OFF;
		TIM_SetCompare2(TIM1,9999);
		Target_Headlight_Brightness = 0;
		Current_Headlight_Brightness = 0;
		return;
	}

	if ((Target_Headlight_Brightness != 0) || (Current_Headlight_Brightness != 0)) {
		if (Current_Headlight_Brightness < Target_Headlight_Brightness) {
			Current_Headlight_Brightness += 3;
			if (Current_Headlight_Brightness > Target_Headlight_Brightness)
				Current_Headlight_Brightness = Target_Headlight_Brightness;
		}
		else if (Current_Headlight_Brightness > Target_Headlight_Brightness) {
			Current_Headlight_Brightness -= 3;
			if (Current_Headlight_Brightness < Target_Headlight_Brightness)
				Current_Headlight_Brightness = Target_Headlight_Brightness;
		}
		Set_Headlights_Brightness(Current_Headlight_Brightness);
	}

	// Set new target
	int new_brightness = Target_Headlight_Brightness;
	if ((data.state < RUNNING_FLYWHEEL) || (ADC1_Val > 2) || (ADC2_Val > 2)) {
		if (lcmConfig.isSet) {
			new_brightness = lcmConfig.headlightBrightness;
		}
		else {
			if (Light_Profile >= LIGHT_PROFILE_1 && Light_Profile <= LIGHT_PROFILE_3) {
				new_brightness = headlight_brightnesses[Light_Profile - 1];
			}
		}
		new_brightness *= data.isForward ? 1 : -1;
	}
	else {
		new_brightness = 0;
		if (lcmConfig.isSet) {
			new_brightness = lcmConfig.headlightIdleBrightness;
			float pitch = data.pitch > 0 ? data.pitch : -data.pitch;
			if ((pitch > 75) && (pitch < 105)) {
				// headlights off when the board is upgright (e.g. being carried or leaning against a wall)
				new_brightness = 0;
			}
		}

		if (light_profile_last == Light_Profile && Flashlight_Detection_Time >= 3100) {
			Flashlight_Detection_Time = 3100;
		}
		else {
			// User double-pressed the power button, show the new brightness when idle
			Flashlight_Detection_Time = 0;
			if (Light_Profile >= LIGHT_PROFILE_1 && Light_Profile <= LIGHT_PROFILE_3 && !lcmConfig.isSet) {
				new_brightness = headlight_brightnesses[Light_Profile - 1];
			}
			light_profile_last = Light_Profile;
		}
	}
	if (new_brightness != Target_Headlight_Brightness) {
		Target_Headlight_Brightness = new_brightness;
		Flashlight_Time = 0;
	}
}

#ifdef USE_BUZZER

/**************************************************
 * @brie   :Buzzer_Task()
 **************************************************/
void Buzzer_Task(void)
{
	static uint8_t buzzer_step = 0;
	static uint8_t light_profile_last = 0;
	static uint8_t ring_frequency = 0;
	static uint16_t sound_frequency = 0;

	if(Power_Flag != PWR_FLAG_BOOTED || Buzzer_Flag == BUZZER_FLAG_SINGLE)
	{
		BUZZER_OFF;
		buzzer_step = 0;
		return;
	}
	
	if(Buzzer_Frequency == 0 && light_profile_last == Light_Profile)
	{
		BUZZER_OFF;
		buzzer_step = 0;
		return;
	}
	
	if(Buzzer_Frequency != 0)	
	{
		switch(buzzer_step)
		{
			case 0:
				sound_frequency = (uint16_t)(-2.78*Buzzer_Frequency+666);
				Buzzer_Time = 0;
				buzzer_step = 1;
			break;
			
			case 1:
				Buzzer_Ring(sound_frequency>>2);
				buzzer_step = 2;
			break;
			
			case 2:
				if(Buzzer_Time > sound_frequency)
				{	
					buzzer_step = 0;
				}
			break;
				
			default:
			
		  break;
		}
	}
	else
	{
		switch(buzzer_step)
		{
			case 0:
				Buzzer_Time = 0;
				buzzer_step = 1;
			break;
			
			case 1:
				Buzzer_Ring(200);
				buzzer_step = 2;
			break;
			
			case 2:
				if(Buzzer_Time > 400)
				{	
					ring_frequency++;
					buzzer_step = 0;
					if(ring_frequency >= Light_Profile)
					{
						ring_frequency = 0;
						light_profile_last = Light_Profile;
					}
					
				}
			break;
				
			default:
			
		  break;
		}
	}
}

#endif

/**************************************************
 * @brie   :Usart_Task()
 **************************************************/
void Usart_Task(void)
{
	static uint8_t usart_step = 0;
	static uint8_t commandIndex = 0; // Store a rotating index so we can implement relevant frequencies of commands
	uint8_t result;

	if(Power_Flag != PWR_FLAG_BOOTED)
	{
		// legacy/motor data
		data.rpm = 0;
		data.dutyCycleNow = 0;
		data.avgInputCurrent = 0;
		//data.inpVoltage = 0;

		// float package data
		data.floatPackageSupported = false;
		data.isOldPackage = false;
		data.state = 255;
		data.fault = 0;
		data.isForward = true;

		usart_step = 0;
		commandIndex = 0;
		
		return;
	}
	
	switch(usart_step)
	{
		case 0:
			// Try the custom app command for the first 2 seconds then fall back to generic GET_VALUES
			if ((data.floatPackageSupported == false) && (Power_Time > VESC_BOOT_TIME * 2)) {
				Get_Vesc_Pack_Data(COMM_GET_VALUES);
			} else {
				uint8_t command = COMM_CUSTOM_APP_DATA;

#ifdef ADV
				if (commandIndex % 20 == 0) {
					// Sending charge info every 20th frame
					command = COMM_CHARGE_INFO;
				} else 
#endif
				if (lcmConfig.debug && commandIndex % 2 == 0) {
					// Send debug info every 2nd frame if enabled
					command = COMM_CUSTOM_DEBUG;
				}

				Get_Vesc_Pack_Data(command);

				if (commandIndex == 255) {
					commandIndex = 0;
				} else {
					commandIndex++;
				}
			}

			usart_step = 1;
		break;
		
		case 1:
			if(VESC_RX_Flag == 1)
			{
				VESC_RX_Flag = 0;
				result = Protocol_Parse(VESC_RX_Buff);
				
				Vesc_Data_Ready = (result == 0);
				Usart_Time = 0;
				usart_step = 2;
			}
			else
			{
				usart_step = 3;
				Usart_Time = 0;
			}
		break;
			
		case 2:
			if(Usart_Time >= 100)
			{
				usart_step = 0;
			}				
		break;
			
		case 3:
			if(VESC_RX_Flag == 1)
			{
				usart_step = 1;
			}
			else if(Usart_Time >= 100)
			{
				usart_step = 0;
			}
		break;
			
		default:
			
		break;
		
	}
	
}

/**************************************************
 * @brie   :ADC_Task()
 * @note   :Eavluate the 3 ADCs: 2 x sensors and 1 x current/voltage (multiplexed via LED1)
 **************************************************/
void ADC_Task(void)
{
	static uint8_t adc_step = 0;
	static uint16_t adc_charge_sum_ave = 0;
	static uint16_t adc1_val_sum_ave = 0;
	static uint16_t adc2_val_sum_ave = 0;
	
	switch(adc_step)
	{
		case 0:
			ADC_Time = 0;
			adc_step = 1;
		break;
		
		case 1:
			if(ADC_Time>100)
			{	
				ADC_Time = 0;
				
				adc_charge_sum_ave= Read_ADC_Value(3);
				adc1_val_sum_ave  = Read_ADC_Value(1);
				adc2_val_sum_ave  = Read_ADC_Value(2);
				
				ADC1_Val = (float)(adc1_val_sum_ave*0.0012890625);
				ADC2_Val = (float)(adc2_val_sum_ave*0.0012890625);
				
				if(V_I == 0)
				{
					if(Charge_Time>100)
					{
						Charge_Current = (float)(-0.008056640625*adc_charge_sum_ave+16.5);
					}
				}
				else
				{
					if(Charge_Time>100)
					{
						Charge_Voltage = (float)(adc_charge_sum_ave*0.0257080078125);
					}
				}
			}
			
		break;
			
	  default:
			
		break;
	}
}

/**************************************************
 * @brie   :VESC_State_Task()
 * @note   :proces VESC state when power is on and we're not charging
 **************************************************/
void VESC_State_Task(void)
{
	if ((Charge_Flag > CHG_FLAG_INITIAL) || (Power_Flag != PWR_FLAG_BOOTED) || !Vesc_Data_Ready)
		return;

	Vesc_Data_Ready = false;

	// Not charging? Get voltage from VESC
	if (data.inpVoltage > 0) {
  	CheckPowerLevel((data.inpVoltage + 1) / BATTERY_PACK_SERIES);
  }

	if(data.dutyCycleNow < 0)
	{
		data.dutyCycleNow = -data.dutyCycleNow;
	}

	// Duty Cycle beep
	if ((lcmConfig.dutyBeep > 0) && (data.dutyCycleNow >= lcmConfig.dutyBeep))
	{
		Buzzer_Frequency = ((((uint8_t)(data.dutyCycleNow))*4)-220);
	}
	else
	{
		Buzzer_Frequency = 0;
	}
	// Don't buzz in wheel slip or flywheel mode
	if (data.state > RUNNING_UPSIDEDOWN) {
		Buzzer_Frequency = 0;
	}
	
	if(data.rpm<0)
	{
		data.rpm = -data.rpm;
	}
	
	if(data.state == RUNNING_FLYWHEEL) {
		Lightbar_Display_Flag = LIGHTBAR_MODE_NO_BATTERY;
		Footpad_Flag = FOOTPAD_FLAG_FLYWHEEL;
		Buzzer_Frequency = 0;
	}
	else if(data.rpm<VESC_RPM)
	{
		if(ADC1_Val < 2.0 && ADC2_Val < 2.0)
		{
			if((data.state == 0) || (data.state >= FAULT_ANGLE_PITCH))
			{   // disengaged board
				Lightbar_Display_Flag = LIGHTBAR_MODE_BATTERY;
			}
		}
		else {
			Lightbar_Display_Flag = LIGHTBAR_MODE_NO_BATTERY;
			if(ADC1_Val > 2.9 && ADC2_Val > 2.9)
			{
				Footpad_Flag = FOOTPAD_FLAG_BOTH;
			}
			else if(ADC1_Val > 2.9)
			{
				Footpad_Flag = FOOTPAD_FLAG_LEFT;
			}
			else if (ADC2_Val > 2.9)
			{
				Footpad_Flag = FOOTPAD_FLAG_RIGHT;
			}
		}
	}
	else
	{
		// Add check for low voltage to force voltage display on WS2812!
		Lightbar_Display_Flag = LIGHTBAR_MODE_NO_BATTERY;
		Footpad_Flag = FOOTPAD_FLAG_RIDING;	// Normal Riding!
	}
	
	// No movement and no ADCs? Shutdown after timeout (10-30min)
	if(ADC1_Val > 2.0 || ADC2_Val > 2.0 || data.rpm > 10)
	{
		Shutdown_Time_S = 0;
		Shutdown_Time_M = 0;
	}
	
	if(Shutdown_Time_S>60000)
	{
		Shutdown_Time_S = 0;
		
		Shutdown_Time_M++;
		if(Shutdown_Time_M >= AUTO_SHUTDOWN_TIME)//lcmConfig.autoShutdownTime)
		{
			Power_Flag = PWR_FLAG_START_POWEROFF;
			Power_Time = 0;
		}
	}

	if(((Shutdown_Time_M > 0) || (Shutdown_Time_S >= 10000)) && (lcmConfig.boardOff))
	{
		// After 10 seconds of idle we allow the board to be shut down via app
		Power_Flag = PWR_FLAG_START_POWEROFF;
		Power_Time = 0;
	}
	lcmConfig.boardOff = false;
}
