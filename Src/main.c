
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l476xx.h"
#include "core_cm4.h"
#include "stm32l4xx_ll_cortex.h"
// #if defined(USE_FULL_ASSERT)
// #include "stm32_assert.h"
// #endif /* USE_FULL_ASSERT */

#include "gpio.h"

void     SystemClock_Config(void);
void 	 LL_Init10msTick(uint32_t HCLKFrequency);

int periodLED = 0; //Période en ms
int counter = 0;
int expe = 1;
int mode = 0;
int main(void)
 {
/* Configure the system clock */
SystemClock_Config();


// config GPIO
GPIO_init();

// init timer pour utiliser la fonction LL_mDelay() de stm32l4xx_ll_utils.c
//LL_Init10msTick( SystemCoreClock );

SysTick_Config(SystemCoreClock/100);

LED_GREEN(0);
/*while (1)
 	{
	if	( BLUE_BUTTON() )
		LED_GREEN(1);
	else {
		LED_GREEN(0);
		LL_mDelay(950);
		LED_GREEN(1);
		LL_mDelay(50);
		}
	}
}*/


while(1){
if (BLUE_BUTTON() && mode==0){
	mode = 1;
}
if (mode){
	LL_LPM_EnableSleep();
	__WFI();
}
}
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */

}
void SystemClock_Config(void)
{
/* MSI configuration and activation */

LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
LL_RCC_MSI_EnableRangeSelection();
if (LL_RCC_MSI_IsEnabledRangeSelect())
	LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_9);
//LL_RCC_MSI_DisablePLLMode();
LL_RCC_MSI_Enable();
//while	(LL_RCC_MSI_IsReady() != 1)
//	{ };
  
/* Main PLL configuration and activation */
//LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
//LL_RCC_PLL_Disable();
//LL_RCC_PLL_DisableDomain_SYS();

/*while(LL_RCC_PLL_IsReady() != 1)
	{ };*/
  
/* Sysclk activation on the main PLL */
LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
//LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
//while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
//	{ };
  
/* Set APB1 & APB2 prescaler*/
LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

/* Update the global variable called SystemCoreClock */
SystemCoreClockUpdate();
}






void LL_Init10msTick(uint32_t HCLKFrequency) {

	LL_InitTick(HCLKFrequency, 10000U);
	/*SysTick->CTRL = LL_SYSTICK_EnableIT;
	SCB->SHP = 1;*/


}

void SysTick_Handler () {
	switch (expe){

	case 1:

		if (counter < 5 * expe) {
			LED_GREEN(1);
			counter++;
		}
		else if (counter < 100) {
			LED_GREEN(0);
			if (counter == 99) {
				counter = 0;
			}
			else {
				counter++;
			}
		}

		break;
	/*case 2 :
		if	( BLUE_BUTTON() ) {

			LL_LPM_EnableSleep();

		}
		else {
			LL_LPM_DisableSleepOnExit();
		}



		if ( counter == periodLED / 20 ) {
			counter = 0;

			LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_5);

		}
		else {

			counter++;
		}
		break;
	default:
		break;*/

	}
}



