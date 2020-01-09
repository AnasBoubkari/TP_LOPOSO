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

/* Systick Initialization */
void LL_Init10msTick(uint32_t HCLKFrequency);

void Configure_RTC_Clock(void);
void Configure_RTC(void);
void Configure_RTC_Calendar(void);

int counter = 0;
int counter_pin = 0;
int expe = 0;
int blue_mode = 0;
int isReady = 0;

int main(void) {

	// config GPIO
	GPIO_init();

	// 1. Initialiser RTC
	Configure_RTC_Clock();

	// 2. Gestion de la variable "expe"
	expe = LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0);
	expe++;
	expe = (expe > 8 ? 1 : expe);
	LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, expe);

	// 3. Configurer horloge
	SystemClockConfig(expe);

	// 4. Activer calibration MSI vs LSE
	if (expe == 8) {
		LL_RCC_MSI_EnablePLLMode();
		LL_LPM_EnableSleep();
		RTC_wakeup_init_from_standby_or_shutdown(20);
	}

	if ((5 <= expe) || (expe >= 7)) {
		LL_RCC_MSI_EnablePLLMode();
		LL_LPM_EnableSleep();
		RTC_wakeup_init_from_stop(20);
	}

	// 5. Configurer timer SysTick
	LL_Init10msTick(SystemCoreClock);

	SysTick_Config(SystemCoreClock / 100);

	LED_GREEN(0);

	while (1)
	{
	}
}

void LL_Init10msTick(uint32_t HCLKFrequency) {

	LL_InitTick(HCLKFrequency, 10000U);

}

void SysTick_Handler() {

	// LED
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

	// PIN_10
	if (counter_pin == 0) {
		counter_pin++;
		LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_10);

	} else {
		counter_pin = 0;
		LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_10);
	}


	// BLUE_MODE
	if (BLUE_BUTTON()) {
		blue_mode = 1;
	}

	if (blue_mode) {
		switch (expe) {
		case 1:
			LL_LPM_EnableSleep();
			__WFI();
			break;
		case 2:
			if (isReady == 0) {
				isReady = 1;
				LL_RCC_MSI_EnablePLLMode();
			}
			break;
		case 3:
			LL_LPM_EnableSleep();
			__WFI();
			break;
		case 4:
			if (isReady == 0) {
				isReady = 1;
				LL_RCC_MSI_EnablePLLMode();
			}
			break;
		case 5:
			if (isReady == 0) {
				isReady = 1;
				LL_PWR_SetPowerMode(LL_PWR_MODE_STOP0);
				LL_LPM_EnableDeepSleep();
				__WFI();
			}
			break;
		case 6:
			if (isReady == 0) {
				isReady = 1;
				LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
				LL_LPM_EnableDeepSleep();
				__WFI();
			}
			break;
		case 7:
			if (isReady == 0) {
				isReady = 1;
				LL_PWR_SetPowerMode(LL_PWR_MODE_STOP2);
				LL_LPM_EnableDeepSleep();
				__WFI();
			}
			break;
		case 8:
			LL_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
			LL_LPM_EnableDeepSleep();
			__WFI();
			break;
		default:
			break;
		}
	}
}

void Configure_RTC_Clock(void) {

	// Démarrage à chaud
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	LL_PWR_EnableBkUpAccess();

	// Démarrage à froid
	if (LL_RCC_LSE_IsReady() != 1) {

		/* Enable LSE */
		LL_RCC_LSE_Enable();
		while (LL_RCC_LSE_IsReady() != 1)
		{
		}

		if (LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE) {
			LL_RCC_ForceBackupDomainReset();
			LL_RCC_ReleaseBackupDomainReset();
			LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
		}

		Configure_RTC();
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
