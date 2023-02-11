/*******************************************************************************
 * scheduler.c
 * Date:        10-02-2022
 * Author:      Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
 * Description: This file has oscillators related information
 *
 *
 *
 ******************************************************************************/

#include "scheduler.h"
#include "em_core.h"
// scheduler routine to set a scheduler event

typedef enum
{
  evtReadTemperature = 1
}eventstriggered;

uint16_t myEvent=0;//Events that are triggered

/************
 *@Function void schedulerSetEventTemperaturemeasurement()
 *@Description Schedules Temperature measurement
 *@Param NULL
 *@Return NULL
 */
void schedulerSetEventTemperaturemeasurement()
{

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  myEvent |=evtReadTemperature;
  CORE_EXIT_CRITICAL();
} // schedulerSetEventXXX()


/************
 *@Function void getNextEvent()
 *@Description Obtain Next Event which is triggered
 *@Param NULL
 *@Return NULL
 */
uint32_t getNextEvent()
{
 uint32_t theEvent;
 //IRQ Disabled
 CORE_DECLARE_IRQ_STATE;
 CORE_ENTER_CRITICAL();
 theEvent = myEvent; // 1 event to return to the caller
 myEvent &= ~(1<<(evtReadTemperature-1));//Clear the Event from myEvent
 CORE_EXIT_CRITICAL();

 // enter critical section
 // clear the event in your data structure, this has to be a read-modify-write
 // exit critical section
 return theEvent;
}
