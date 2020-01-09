/*
 * clock.h
 *
 *  Created on: 5 nov. 2019
 *      Author: elboubka
 */

#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_rtc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_exti.h"

#ifndef SRC_CLOCK_H_
#define SRC_CLOCK_H_


void SystemClock_Config4MHz_PLL80MHz(void);
void SystemClock_Config24MHz_LATENCY1(void);
void SystemClock_Config24MHz_LATENCY3(void);

void SystemClockConfig(int expe);
void InitCalibration_Sleep(int expe);

void RTC_wakeup_init_from_standby_or_shutdown( int delay );
void RTC_wakeup_init_from_stop( int delay );

#endif /* SRC_CLOCK_H_ */
