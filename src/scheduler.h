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
  PRIMARY_BUTTON_CHAR_UUID,
  DISCOVER_BUTTON_CHAR_UUID,
  SET_CHAR_BUTTON_NOTIFY,
  DISCOVERY_COMPLETE,
  CLOSE_CONNECTION
}discovery_state;
typedef enum
{
  evtReadAmbientsensor = 1,
  evti2ccomp1setcomplete = 2,
  evti2ctransfercomplete = 4,
  evtgpiopb0intset = 8,
  evtgpiopb0intclear = 16,
  evtgpiopb1intset = 32,
  evtgpiopb1intclear = 64
}eventstriggered;
// Health Thermometer service UUID defined by Bluetooth SIG
static const uint8_t thermo_service[2] = { 0x09, 0x18 };
static const uint8_t push_button_service[16] = { 0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x01, 0x00, 0x00, 0x00 };
// Temperature Measurement characteristic UUID defined by Bluetooth SIG
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };
static const uint8_t push_button_char[16] = { 0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x02, 0x00, 0x00, 0x00 };
void schedulerSetEventAmbiencemeasurement();
uint32_t getNextEvent();
void schedulerSetEventcomp1set();
void schedulerSetEventi2cTransferDone();
void schedulerSetEventGPIOPB1clear();
void schedulerSetEventGPIOPB1set();
void schedulerSetEventGPIOPB0set();
void schedulerSetEventGPIOPB0clear();
//#if DEVICE_IS_BLE_SERVER
//void temperature_state_machine(sl_bt_msg_t *);
void ambient_light_state_machine(int);
//#else
//void discovery_state_machine(sl_bt_msg_t *);
//#endif
#endif /* SRC_SCHEDULER_H_ */
