/*******************************************************************************
 * ble.h
 * Date:        25-02-2022
 * Author:      Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
 * Description: This file has BLE related information
 *
 *
 *
 ******************************************************************************/

#ifndef SRC_BLE_H_
#define SRC_BLE_H_

#include <stdio.h>
#include <stdint.h>
#include "sl_bt_api.h"
#include "sl_status.h"
#include "sl_bluetooth.h"
#include <stdbool.h>


#define UINT8_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); }

#define UINT32_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
 *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }

#define UINT32_TO_FLOAT(m, e) (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))
// BLE Data Structure, save all of our private BT data in here.
// Modern C (circa 2021 does it this way)
// typedef ble_data_struct_t is referred to as an anonymous struct definition
typedef struct {
  // values that are common to servers and clients
  bd_addr       myAddress;
  uint8_t       myAddressType;
  uint8_t       connection_handle;
  uint8_t       ble_connection_handle;
  // values unique for server
  // The advertising set handle allocated from Bluetooth stack.
  uint8_t       advertisingSetHandle;
  uint8_t       temperatureSetHandle;
  bool          flag_connection_open_close;            // true when in an open connection
  bool          flag_ok_to_send_htm_indications; // true when client enabled indications
  bool          flag_in_flight;
  // values unique for client
} ble_data_struct_t;

ble_data_struct_t* getBleDataPtr(void);
void handle_ble_event(sl_bt_msg_t *);
void ble_send_temp(uint32_t);
#endif /* SRC_BLE_H_ */
