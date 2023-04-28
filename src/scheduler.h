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
  PRIMARY_PIR_CHAR_UUID,
  DISCOVER_PIR_CHAR_UUID,
  SET_CHAR_PIR_NOTIFY,
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
//// Health Thermometer service UUID defined by Bluetooth SIG
//static const uint8_t thermo_service[2] = { 0x09, 0x18 };
//static const uint8_t push_button_service[16] = { 0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x01, 0x00, 0x00, 0x00 };
//// Temperature Measurement characteristic UUID defined by Bluetooth SIG
//static const uint8_t thermo_char[2] = { 0x1c, 0x2a };
//static const uint8_t push_button_char[16] = { 0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x02, 0x00, 0x00, 0x00 };
//Luminisense service UUID defined by Bluetooth SIG
static const uint8_t lux_char[16] = { 0x93, 0x34, 0xb3, 0xd9, 0x00, 0x91, 0xdc, 0x94, 0x84, 0x48, 0xc2, 0x33, 0x3d, 0x9a, 0x9f, 0xbb};
static const uint8_t pir_char[16] = { 0xdb, 0xc2, 0x2c, 0xc4, 0x55, 0x7a, 0x48, 0xaf, 0x46, 0x4e, 0x83, 0xe3, 0xb2, 0x1d, 0xdb, 0xfc};

// Temperature Measurement characteristic UUID defined by Bluetooth SIG
static const uint8_t lux_service[16] = { 0xd4, 0x7f, 0x7d, 0x93, 0xd2, 0x71,  0x60, 0x82, 0x84, 0x44, 0x72, 0xe9, 0x9a, 0xdb, 0xf2, 0xea};
static const uint8_t pir_service[16] = { 0x52 , 0x5a, 0x4c, 0x57, 0xee, 0x04, 0xea, 0xa3, 0x5b, 0x46, 0x9d, 0x88, 0x97, 0xcc, 0xe8, 0xbd };
void schedulerSetEventAmbiencemeasurement();
uint32_t getNextEvent();
void schedulerSetEventcomp1set();
void schedulerSetEventi2cTransferDone();
void schedulerSetEventGPIOPB1clear();
void schedulerSetEventGPIOPB1set();
void schedulerSetEventGPIOPB0set();
void schedulerSetEventGPIOPB0clear();
void schedulerSetCountPIR_1_detect();
void schedulerSetCountPIR_2_detect();
//#if DEVICE_IS_BLE_SERVER
//void temperature_state_machine(sl_bt_msg_t *);
void ambient_light_state_machine(sl_bt_msg_t *evt);
//#else
//void discovery_state_machine(sl_bt_msg_t *);
//#endif
#endif /* SRC_SCHEDULER_H_ */
