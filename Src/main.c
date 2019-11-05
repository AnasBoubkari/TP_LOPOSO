/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l476xx.h"
#include "core_cm4.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_rtc.h"
#include <stdio.h>
#include "clock.h"
#include "gpio.h"

/* ck_apre=LSIFreq/(ASYNC prediv + 1) with LSIFreq=32 kHz RC */
#define RTC_ASYNCH_PREDIV          ((uint32_t)0x7F)
/* ck_spre=ck_apre/(SYNC prediv + 1) = 1 Hz */
#define RTC_SYNCH_PREDIV           ((uint32_t)0x00F9)

#define RTC_ERROR_NONE    0
#define RTC_ERROR_TIMEOUT 1

/* Define used to indicate date/time updated */
#define RTC_BKP_DATE_TIME_UPDTATED ((uint32_t)0x32F2)

/* Clock configurations */
void SystemClock_Config24MHz(void);
void SystemClock_Config4MHz_PLL80MHz(void);

/* Systick Initialization */
void LL_Init10msTick(uint32_t HCLKFrequency);

void Configure_RTC_Clock(void);
void Configure_RTC(void);
void Configure_RTC_Calendar(void);
void Show_RTC_Calendar(void);

int periodLED = 0; //Période en ms
int counter = 0;
int expe = 0;
int mode = 0;

/* Buffers used for displaying Time and Date */
uint8_t aShowTime[50] = { 0 };
uint8_t aShowDate[50] = { 0 };

int main(void) {

	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	LL_PWR_EnableBkUpAccess();

	/* Configuration RTC clock */
	Configure_RTC_Clock(); // LSE

	if (LL_RTC_BAK_GetRegister(RTC,
	LL_RTC_BKP_DR1) != RTC_BKP_DATE_TIME_UPDTATED) {
		/*##-Configure the RTC peripheral #######################################*/
		Configure_RTC();
		/* Configure RTC Calendar */
		/*Configure_RTC_Calendar();*/
	}

	expe = LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0);
	expe++;

	/* Configure the system clock */
	SystemClockConfig(expe);

	/* Calibration */
	InitCalibration_Sleep(expe);

	// config GPIO
	GPIO_init();

	// init timer pour utiliser la fonction LL_mDelay() de stm32l4xx_ll_utils.c
	//LL_Init10msTick( SystemCoreClock );

	SysTick_Config(SystemCoreClock / 100);

	LED_GREEN(0);

	while (1) {

		//Show_RTC_Calendar();

		if (BLUE_BUTTON() && mode == 0) {
			mode = 1;
		}
		if (mode) {

			LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, expe);

			if (expe == 1) {
				LL_LPM_EnableSleep();
				__WFI();
			} else if (expe == 2) {
				LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
				LL_PWR_EnableBkUpAccess();
				if (LL_RCC_LSE_IsReady() == 0) {
					LL_RCC_ForceBackupDomainReset();
					LL_RCC_ReleaseBackupDomainReset();
					LL_RCC_LSE_Enable();
				}

				//LL_RCC_ReleaseBackup
				while (!LL_RCC_LSE_IsReady())
					;

				LL_RCC_MSI_EnablePLLMode();
			}
		}
	}

}

void LL_Init10msTick(uint32_t HCLKFrequency) {

	LL_InitTick(HCLKFrequency, 10000U);

}

void SysTick_Handler() {

	switch (expe) {

	case 1:

		if (counter < 5 * expe) {
			LED_GREEN(1);
			counter++;
		} else if (counter < 100) {
			LED_GREEN(0);
			if (counter == 99) {
				counter = 0;
			} else {
				counter++;
			}
		}

		break;

	case 2:

		if (counter == 0) {
			counter++;
			LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_10);

		} else {
			counter = 0;
			LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_10);
		}

		break;

	default:
		break;

	}
}

void Configure_RTC_Clock(void) {

	if (LL_RCC_LSE_IsReady() != 1) {

		/* Enable LSE */
		LL_RCC_LSE_Enable();
		while (LL_RCC_LSE_IsReady() != 1)
			;

		if (LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE) {
			LL_RCC_ForceBackupDomainReset();
			LL_RCC_ReleaseBackupDomainReset();
			LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
		}

	}
}

/**
 * @brief  Configure RTC.
 * @note   Peripheral configuration is minimal configuration from reset values.
 *         Thus, some useless LL unitary functions calls below are provided as
 *         commented examples - setting is default configuration from reset.
 * @param  None
 * @retval None
 */
void Configure_RTC(void) {
	/*##-1- Enable RTC peripheral Clocks #######################################*/
	LL_RCC_EnableRTC();

	/*##-2- Disable RTC registers write protection ##############################*/
	LL_RTC_DisableWriteProtection(RTC);

	/*##-3- Enter in initialization mode #######################################*/
	LL_RTC_EnableInitMode(RTC);

	while (LL_RTC_IsActiveFlag_INIT(RTC) != 1) {
	}

	/*##-4- Configure RTC ######################################################*/
	/* Set Hour Format */
	LL_RTC_SetHourFormat(RTC, LL_RTC_HOURFORMAT_AMPM);
	/* Set Asynch Prediv (value according to source clock) */
	LL_RTC_SetAsynchPrescaler(RTC, RTC_ASYNCH_PREDIV);
	/* Set Synch Prediv (value according to source clock) */
	LL_RTC_SetSynchPrescaler(RTC, RTC_SYNCH_PREDIV);
	/* Set OutPut */
	/* Reset value is LL_RTC_ALARMOUT_DISABLE */
	//LL_RTC_SetAlarmOutEvent(RTC, LL_RTC_ALARMOUT_DISABLE);
	/* Set OutPutPolarity */
	/* Reset value is LL_RTC_OUTPUTPOLARITY_PIN_HIGH */
	//LL_RTC_SetOutputPolarity(RTC, LL_RTC_OUTPUTPOLARITY_PIN_HIGH);
	/* Set OutPutType */
	/* Reset value is LL_RTC_ALARM_OUTPUTTYPE_OPENDRAIN */
	//LL_RTC_SetAlarmOutputType(RTC, LL_RTC_ALARM_OUTPUTTYPE_OPENDRAIN);
	/*##-5- Exit of initialization mode #######################################*/
	LL_RTC_DisableInitMode(RTC);
	/* Clear RSF flag */
	LL_RTC_ClearFlag_RS(RTC);

	while (LL_RTC_IsActiveFlag_RS(RTC) != 1) {
	}

	/*##-6- Enable RTC registers write protection #############################*/
	LL_RTC_EnableWriteProtection(RTC);
}

/**
 * @brief  Configure the current time and date.
 * @param  None
 * @retval None
 */
void Configure_RTC_Calendar(void) {
	/*##-1- Disable RTC registers write protection ############################*/
	LL_RTC_DisableWriteProtection(RTC);

	/*##-2- Enter in initialization mode ######################################*/
	LL_RTC_EnableInitMode(RTC);

	while (LL_RTC_IsActiveFlag_INIT(RTC) != 1) {
	}

	/*##-3- Configure the Date ################################################*/
	/* Note: __LL_RTC_CONVERT_BIN2BCD helper macro can be used if user wants to*/
	/*       provide directly the decimal value:                               */
	/*       LL_RTC_DATE_Config(RTC, LL_RTC_WEEKDAY_MONDAY,                    */
	/*                          __LL_RTC_CONVERT_BIN2BCD(31), (...))           */
	/* Set Date: Monday March 31th 2015 */
	LL_RTC_DATE_Config(RTC, LL_RTC_WEEKDAY_MONDAY, 0x31, LL_RTC_MONTH_MARCH,
			0x15);

	/*##-4- Configure the Time ################################################*/
	/* Set Time: 11:59:55 PM*/
	LL_RTC_TIME_Config(RTC, LL_RTC_TIME_FORMAT_PM, 0x11, 0x59, 0x55);

	/*##-5- Exit of initialization mode #######################################*/

	LL_RTC_DisableInitMode(RTC);

	/* Clear RSF flag */
	LL_RTC_ClearFlag_RS(RTC);

	while (LL_RTC_IsActiveFlag_RS(RTC) != 1)
		;

	/*##-6- Enable RTC registers write protection #############################*/
	LL_RTC_EnableWriteProtection(RTC);

	/*##-8- Writes a data in a RTC Backup data Register1 #######################*/
	LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR1, RTC_BKP_DATE_TIME_UPDTATED);
}

