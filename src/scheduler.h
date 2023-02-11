/*
 * scheduler.h
 *
 *  Created on: 09-Feb-2023
 *      Author: sriraj
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_
#include "stdint.h"
typedef enum
{
  evtLETimer_UF = 1
}eventInterruptflag;
void schedulerSetEventTemperaturemeasurement();
uint32_t getNextEvent();

#endif /* SRC_SCHEDULER_H_ */
