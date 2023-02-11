/*
 * irq.c
 * Date:        03-02-2022
 * Author:      Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
 * Description: This file has interrupt handler related information
 *
 *
 *
 ******************************************************************************/
#include "irq.h"
#include "em_letimer.h"
#include  <stdbool.h>


bool uf_int = true;//Check if underflow and COMP1 flag is set to control the LED ON and OFF

/*************
 * @Function void LETIMER0_IRQHandler()
 * @Description Used as LETIMER0 Interrupt Handler
 * @Param NULL
 * @Return NULL
 *************/
void LETIMER0_IRQHandler(void)
{
  uint32_t flag_value;//Obtains interrupt flags
  flag_value = LETIMER_IntGet(LETIMER0);//Get flag info
  LETIMER_IntClear(LETIMER0,flag_value);
  //Check if COMP1 interrupt is set
  //Check if UF interrupt is set
 if((flag_value&LETIMER_IF_UF))
  {
     //Schedule the Event Temperature measurement
     schedulerSetEventTemperaturemeasurement();
  }
}
