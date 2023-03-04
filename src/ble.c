/*******************************************************************************
 * ble.c
 * Date:        25-02-2022
 * Author:      Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
 * Description: This file has BLE related information
 *
 *
 *
 ******************************************************************************/
#include "src/ble.h"
#include "sl_bt_api.h"
#include "gatt_db.h"
#include "src/lcd.h"
/*******LOG MACRO AND HEADER*******/
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"
/********************************/

#define ADV_MAX_INTERVAL 400   //(250/0.625)
#define ADV_MIN_INTERVAL 400  //(250/0.625)
#define ADV_DURATION 0  //default value
#define ADV_MAX_EVENTS 0 //default value
#define MIN_CE_LENGTH            0  //default value
#define MAX_CE_LENGTH            0xffff //default value
#define MIN_INTERVAL             60
#define MAX_INTERVAL             60
#define LATENCY        4
#define TIMEOUT                  800  //1000ms


ble_data_struct_t ble_data;
sl_status_t sc;

ble_data_struct_t* getBleDataPtr() {
  return (&ble_data);
} // getBleDataPtr()

/**************************************************************************//**
 * @Function: handle_ble_event()
 * @Description: Handling Bluetooth events.
 * @Param sl_bt_msg_t *
 * @Return NULL
 *****************************************************************************/
void handle_ble_event(sl_bt_msg_t *evt) {

  ble_data_struct_t *bleDataPtr = getBleDataPtr();
  switch (SL_BT_MSG_ID(evt->header))
  {
    // ******************************************************
    // Events common to both Servers and Clients
    // ******************************************************
    // --------------------------------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack API commands before receiving this boot event!
    // Including starting BT stack soft timers!
    // --------------------------------------------------------
    case sl_bt_evt_system_boot_id://Opening advertising connection and Initializing advertising
      // handle boot event
      displayInit();
      displayPrintf(DISPLAY_ROW_NAME, "Server");
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A6");
      sc = sl_bt_system_get_identity_address(&bleDataPtr->myAddress, &bleDataPtr->myAddressType);
      if(sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_system_get_identity_address() returned != 0 status=0x%04x", (unsigned int) sc);
          break;
        }
      else
        {
          displayPrintf(DISPLAY_ROW_BTADDR, "%x:%x:%x:%x:%x:%x",
                        bleDataPtr->myAddress.addr[0], bleDataPtr->myAddress.addr[1],
                        bleDataPtr->myAddress.addr[2], bleDataPtr->myAddress.addr[3],
                        bleDataPtr->myAddress.addr[4], bleDataPtr->myAddress.addr[5]);
        }
      sc=sl_bt_advertiser_create_set(&bleDataPtr->advertisingSetHandle);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_create_set() returned != 0 status=0x%04x", (unsigned int) sc);
          break;
        }
      sc=sl_bt_advertiser_set_timing(bleDataPtr->advertisingSetHandle,ADV_MAX_INTERVAL,ADV_MIN_INTERVAL,ADV_DURATION,ADV_MAX_EVENTS);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_set_timing() returned != 0 status=0x%04x", (unsigned int) sc);
          break;
        }
      sc=sl_bt_advertiser_start(bleDataPtr->advertisingSetHandle,sl_bt_advertiser_general_discoverable,sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x", (unsigned int) sc);
        }
      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");

      break;
    case sl_bt_evt_connection_opened_id://Connection established
      bleDataPtr->ble_connection_handle=evt->data.evt_connection_opened.connection;
      sl_bt_advertiser_stop(bleDataPtr->advertisingSetHandle);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_stop() returned != 0 status=0x%04x", (unsigned int) sc);
          break;
        }
      sc=sl_bt_connection_set_parameters(bleDataPtr->ble_connection_handle,MIN_INTERVAL,MAX_INTERVAL,LATENCY,TIMEOUT,MIN_CE_LENGTH,MAX_CE_LENGTH);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_connection_set_parameters() returned != 0 status=0x%04x", (unsigned int) sc);
          break;
        }
      else
        {
          bleDataPtr->flag_connection_open_close = true;
          displayPrintf(DISPLAY_ROW_CONNECTION, "Connected" );
        }
      break; // handle open event
    case sl_bt_evt_connection_closed_id://Closing Connection. Happens when bluetooth is disconnected
      sc=sl_bt_advertiser_start(bleDataPtr->advertisingSetHandle,sl_bt_advertiser_general_discoverable,sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x", (unsigned int) sc);
          break;
        }
      else
        {
          displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising" );
        }
      // handle close events
      bleDataPtr->flag_ok_to_send_htm_indications = false;
      bleDataPtr->flag_connection_open_close = false;
      bleDataPtr->flag_in_flight = false;
      displayPrintf(DISPLAY_ROW_TEMPVALUE,"");
      break;
    case sl_bt_evt_connection_parameters_id://printing the connection values provided by the Client
      LOG_INFO("Interval = %d\n\r Latency = %d\n\r Timeout = %d\n\r",(int)((evt->data.evt_connection_parameters.interval)*1.25),
               (int)(evt->data.evt_connection_parameters.latency),
               (int)((evt->data.evt_connection_parameters.timeout)));
      break;
    case sl_bt_evt_gatt_server_characteristic_status_id://Verify if data is updated in GATT server and ack received from Client
      if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_disable)
        {
          bleDataPtr->flag_ok_to_send_htm_indications = false;
          displayPrintf(DISPLAY_ROW_TEMPVALUE,"");
        }
      else if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_indication)
        {
          if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement)
            {
              bleDataPtr->flag_ok_to_send_htm_indications = true;
            }
        }
      else
        {

        }
      if(evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_confirmation)
        {
          bleDataPtr->flag_in_flight = false;
        }
      break;
    case sl_bt_evt_gatt_server_indication_timeout_id://Check for timeout condition
      if (bleDataPtr->flag_in_flight == true)
        {
          LOG_ERROR("Error in indication\n\r");
        }
      bleDataPtr->flag_in_flight=false;
      break;

    case sl_bt_evt_system_soft_timer_id:
      if(evt->data.evt_system_soft_timer.handle==0)
      {
               displayUpdate();
      }
      break;

      // more case statements to handle other BT events
  } // end - switch
}// handle_ble_event()

/**************************************************************************//**
 * @Function: ble_send_temp()
 * @Description: Handling Temperature
 * @Param uint32_t
 * @Return NULL
 *****************************************************************************/
void ble_send_temp(uint32_t temperature_in_c)
{
  uint8_t htm_temperature_buffer[5];
  uint8_t *p = htm_temperature_buffer;
  uint32_t htm_temperature_flt;
  UINT8_TO_BITSTREAM(p,0); //set n to 0
  htm_temperature_flt = UINT32_TO_FLOAT(temperature_in_c*1000, -3);
  // Convert temperature to bit stream and place it in the htm_temperature_buffer
  UINT32_TO_BITSTREAM(p, htm_temperature_flt);
  ble_data_struct_t *bleDataPtr = getBleDataPtr();
  sc=sl_bt_gatt_server_write_attribute_value(gattdb_temperature_measurement,0,5,htm_temperature_buffer);
  displayPrintf(DISPLAY_ROW_TEMPVALUE, "temp=%d", temperature_in_c);
  if(bleDataPtr->flag_connection_open_close == true  && bleDataPtr->flag_ok_to_send_htm_indications == true)
  {
      if(bleDataPtr->flag_in_flight == false)
      {
          sc = sl_bt_gatt_server_send_indication(bleDataPtr->ble_connection_handle,gattdb_temperature_measurement,5,htm_temperature_buffer);
          if (sc != SL_STATUS_OK)
          {
              LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x", (unsigned int) sc);
          }
          else
          {
              bleDataPtr->flag_in_flight = true;
          }
          LOG_INFO("temp=%d\n\r", temperature_in_c);//print temperature
      }
  }
}
