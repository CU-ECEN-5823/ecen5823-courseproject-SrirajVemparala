/*******************************************************************************
 * oscillators.c
 * Date:        03-02-2022
 * Author:      Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
 * Description: This file has oscillators related information
 *
 *
 *
 ******************************************************************************/

#include "em_cmu.h"

#define ENABLE 1
#define DISABLE 0

/************
 *@Function void clock_init()
 *@Description Clock Initialization
 *@Param NULL
 *@Return NULL
 */
void cmu_init()
{
#if ((LOWEST_ENERGY_MODE == 0) || (LOWEST_ENERGY_MODE == 1) || (LOWEST_ENERGY_MODE == 2))
  CMU_OscillatorEnable(cmuOsc_LFXO,true,true);//Enable Clock
  CMU_ClockSelectSet(cmuClock_LETIMER0,cmuSelect_LFXO);//select LFXO as clock. parameters: CMU_Clock_TypeDef , CMU_Select_TypeDef
  CMU_ClockDivSet(cmuClock_LETIMER0,cmuClkDiv_4);//pre-scalar as 4 CMU_Clock_TypeDef,#define cmuClkDiv_8
#elif LOWEST_ENERGY_MODE  == 3
  CMU_OscillatorEnable(cmuOsc_ULFRCO,true,true);//Enable Clock
  CMU_ClockSelectSet(cmuClock_LETIMER0,cmuSelect_ULFRCO);//select ULFRCO as clock. CMU_Clock_TypeDef , CMU_Select_TypeDef
  CMU_ClockDivSet(cmuClock_LETIMER0,cmuClkDiv_1);//pre-scalar as 1, CMU_Clock_TypeDef,#define cmuClkDiv_1
#endif
  CMU_ClockEnable(cmuClock_LETIMER0,ENABLE);//Enable Clock for LETTIMER0

}
