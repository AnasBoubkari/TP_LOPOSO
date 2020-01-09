/*
 * clock.c
 *
 *  Created on: 5 nov. 2019
 *      Author: elboubka
 */
#include "clock.h"

/**********************************************************************/
/* EXPE1 - MSI Clock Congig 4MHZ + PLL 80MHz*/
/**********************************************************************/
void SystemClock_Config4MHz_PLL80MHz(void) {
	/* MSI configuration and activation */
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
	LL_RCC_MSI_Enable();
	while (LL_RCC_MSI_IsReady() != 1)
	{ };

	/* Main PLL configuration and activation */
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
	LL_RCC_PLL_Enable();
	LL_RCC_PLL_EnableDomain_SYS();
	while(LL_RCC_PLL_IsReady() != 1)
	{ };

	/* Sysclk activation on the main PLL */
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
	{ };

	/* Set APB1 & APB2 prescaler*/
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
	/* Update the global variable called SystemCoreClock */


	SystemCoreClockUpdate();

}

/**********************************************************************/
/* EXPE2 - MSI Clock Config 24MHz */
/**********************************************************************/
void SystemClock_Config24MHz_LATENCY1(void) {

	/* MSI configuration and activation */

	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
	LL_RCC_MSI_EnableRangeSelection();

	if (LL_RCC_MSI_IsEnabledRangeSelect()) {
		LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_9);
	}

	LL_RCC_MSI_Enable();
	while (LL_RCC_MSI_IsReady() != 1)
		;

	/* Sysclk activation on the main PLL */
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

	/* Set APB1 & APB2 prescaler*/
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

	/* Update the global variable called SystemCoreClock */
	SystemCoreClockUpdate();
}

void SystemClock_Config24MHz_LATENCY3(void) {

	/* MSI configuration and activation */

	LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
	LL_RCC_MSI_EnableRangeSelection();

	if (LL_RCC_MSI_IsEnabledRangeSelect()) {
		LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_9);
	}

	LL_RCC_MSI_Enable();
	while (LL_RCC_MSI_IsReady() != 1)
		;

	/* Sysclk activation on the main PLL */
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

	/* Set APB1 & APB2 prescaler*/
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);

	/* Update the global variable called SystemCoreClock */
	SystemCoreClockUpdate();
}



void SystemClockConfig(int expe) {

	if (expe == 1 || expe == 0) {
		SystemClock_Config4MHz_PLL80MHz();
	} else if (expe == 2) {
		SystemClock_Config24MHz_LATENCY1();
	} else if (expe >= 2) {
		SystemClock_Config24MHz_LATENCY3();
	}


}

void Calibration_MSI_LSE_ON(int expe){
	LL_RCC_MSI_EnablePLLMode();

}

void InitCalibration_Sleep(int expe) {

	if (expe >= 1 && expe <= 4) {
		LL_RCC_MSI_DisablePLLMode();
		LL_LPM_DisableSleepOnExit();
	} else if (expe >= 5 && expe <= 8) {
		LL_RCC_MSI_EnablePLLMode();
		LL_LPM_EnableSleepOnExit();
	}

}

// partie commune a toutes les utilisations du wakeup timer
static void RTC_wakeup_init( int delay )
{
	LL_RTC_DisableWriteProtection( RTC );
	LL_RTC_WAKEUP_Disable( RTC );
	while	( !LL_RTC_IsActiveFlag_WUTW( RTC ) )
	{ }
	// connecter le timer a l'horloge 1Hz de la RTC
	LL_RTC_WAKEUP_SetClock( RTC, LL_RTC_WAKEUPCLOCK_CKSPRE );
	// fixer la duree de temporisation
	LL_RTC_WAKEUP_SetAutoReload( RTC, delay );	// 16 bits
	LL_RTC_ClearFlag_WUT(RTC);
	LL_RTC_EnableIT_WUT(RTC);
	LL_RTC_WAKEUP_Enable(RTC);
	LL_RTC_EnableWriteProtection(RTC);
}

// Dans le cas des modes STANDBY et SHUTDOWN, le MPU sera reveille par reset
// causé par 1 wakeup line (interne ou externe) (le NVIC n'est plus alimenté)
void RTC_wakeup_init_from_standby_or_shutdown( int delay )
{
	RTC_wakeup_init( delay );
	// enable the Internal Wake-up line
	LL_PWR_EnableInternWU();	// ceci ne concerne que Standby et Shutdown, pas STOPx
}

// Dans le cas des modes STOPx, le MPU sera reveille par interruption
// le module EXTI et une partie du NVIC sont encore alimentes
// le contenu de la RAM et des registres étant préservé, le MPU
// reprend l'execution après l'instruction WFI
void RTC_wakeup_init_from_stop( int delay )
{
	RTC_wakeup_init( delay );
	// valider l'interrupt par la ligne 20 du module EXTI, qui est réservée au wakeup timer
	LL_EXTI_EnableIT_0_31( LL_EXTI_LINE_20 );
	LL_EXTI_EnableRisingTrig_0_31( LL_EXTI_LINE_20 );
	// valider l'interrupt chez NVIC
	NVIC_SetPriority( RTC_WKUP_IRQn, 1 );
	NVIC_EnableIRQ( RTC_WKUP_IRQn );
}

// wakeup timer interrupt Handler (inutile mais doit etre defini)
void RTC_WKUP_IRQHandler()
{
	LL_EXTI_ClearFlag_0_31( LL_EXTI_LINE_20 );
}
