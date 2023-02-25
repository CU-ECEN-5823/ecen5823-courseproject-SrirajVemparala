/*
 * scheduler.h
 *
 *  Created on: 09-Feb-2023
 *      Author: sriraj
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_
#include "stdint.h"
#include "sl_bt_api.h"

void schedulerSetEventTemperaturemeasurement();
uint32_t getNextEvent();
void temperature_state_machine(sl_bt_msg_t *);
void schedulerSetEventcomp1set();
void schedulerSetEventi2cTransferDone();
#endif /* SRC_SCHEDULER_H_ */
