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
#include "src/ble_device_type.h"
typedef enum
{
  PRIMARY_CHAR_UUID,
  DISCOVER_CHAR_UUID,
  SET_CHAR_NOTIFY,
  DISCOVERY_COMPLETE,
  CLOSE_CONNECTION
}discovery_state;
typedef enum
{
  evtReadTemperature = 1,
  evti2ccomp1setcomplete = 2,
  evti2ctransfercomplete = 4,
  evtgpiopb0intset = 8,
  evtgpiopb0intclear = 16
}eventstriggered;
// Health Thermometer service UUID defined by Bluetooth SIG
static const uint8_t thermo_service[2] = { 0x09, 0x18 };

// Temperature Measurement characteristic UUID defined by Bluetooth SIG
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };

void schedulerSetEventTemperaturemeasurement();
uint32_t getNextEvent();
void schedulerSetEventcomp1set();
void schedulerSetEventi2cTransferDone();
#if DEVICE_IS_BLE_SERVER
void temperature_state_machine(sl_bt_msg_t *);
void schedulerSetEventGPIOPB0set();
void schedulerSetEventGPIOPB0clear();
#else
void discovery_state_machine(sl_bt_msg_t *);
#endif
#endif /* SRC_SCHEDULER_H_ */
