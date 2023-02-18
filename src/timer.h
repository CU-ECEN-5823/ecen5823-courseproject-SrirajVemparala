/*******************************************************************************
 * timers.h
 * Date:        03-02-2022
 * Author:      Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
 * Description: This file has LETIMER0 related information
 *
 *
 *
 ******************************************************************************/

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_
#include "app.h"

#define one_milli_in_microseconds 1000
#define countervalue_milli_in_microseconds 65535000
//#define LETIMER_ON_TIME_MS  175
#define LETIMER_PERIOD_MS 3000

#if ((LOWEST_ENERGY_MODE == 0) || (LOWEST_ENERGY_MODE == 1) || (LOWEST_ENERGY_MODE == 2))
#define CLOCK_FREQUENCY 8192
#define COMP0 (LETIMER_PERIOD_MS*CLOCK_FREQUENCY)/1000
//#define ON_COUNTS (LETIMER_ON_TIME_MS*CLOCK_FREQUENCY)/1000
//#define COMP1 (COMP0 - ON_COUNTS)
#elif(LOWEST_ENERGY_MODE  == 3)
#define CLOCK_FREQUENCY 1000
#define COMP0 (LETIMER_PERIOD_MS*CLOCK_FREQUENCY)/1000
//#define ON_COUNTS (LETIMER_ON_TIME_MS*CLOCK_FREQUENCY)/1000
//#define COMP1 (COMP0 - ON_COUNTS)
#endif
void init_LETIMER0();
void timerdelay(uint32_t);
void timerwaitus_irq(uint32_t);

#endif /* SRC_TIMER_H_ */
