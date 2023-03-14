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
#include "src/ble_device_type.h"
#include "stdint.h"
#include "math.h"
/*******LOG MACRO AND HEADER*******/
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"
/********************************/
#if DEVICE_IS_BLE_SERVER
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
#define SCAN_PASSIVE                  0

#else

#define DIS_MAX_INTERVAL 60   //(250/0.625)
#define DIS_MIN_INTERVAL 60  //(250/0.625)
#define DIS_DURATION 0  //default value
#define SLAVE_LATENCY        3
#define SCAN_SUPERVISION_TIMEOUT 750
#define MIN_CE_LENGTH            0  //default value
#define MAX_CE_LENGTH            4 //default value
#define SCAN_INTERVAL                 80   //50ms
#define SCAN_PASSIVE                  0
#define SCAN_WINDOW                   40

#endif

ble_data_struct_t ble_data;
sl_status_t sc;

ble_data_struct_t* getBleDataPtr() {
  return (&ble_data);
} // getBleDataPtr()

/****************************************/
// -----------------------------------------------
// Private function, original from Dan Walkes. I fixed a sign extension bug.
// We'll need this for Client A7 assignment to convert health thermometer
// indications back to an integer. Convert IEEE-11073 32-bit float to signed integer.
// -----------------------------------------------
#if !(DEVICE_IS_BLE_SERVER)
int32_t temperature_in_c=0;
// -----------------------------------------------
// Private function, original from Dan Walkes. I fixed a sign extension bug.
// We'll need this for Client A7 assignment to convert health thermometer
// indications back to an integer. Convert IEEE-11073 32-bit float to signed integer.
// -----------------------------------------------
static int32_t FLOAT_TO_INT32(const uint8_t *value_start_little_endian)
{
 uint8_t signByte = 0;
 int32_t mantissa;
 // input data format is:
 // [0] = flags byte
 // [3][2][1] = mantissa (2's complement)
 // [4] = exponent (2's complement)
 // BT value_start_little_endian[0] has the flags byte
 int8_t exponent = (int8_t)value_start_little_endian[4];
 // sign extend the mantissa value if the mantissa is negative
 if (value_start_little_endian[3] & 0x80) { // msb of [3] is the sign of the mantissa
 signByte = 0xFF;
 }
 mantissa = (int32_t) (value_start_little_endian[1] << 0) |
 (value_start_little_endian[2] << 8) |
 (value_start_little_endian[3] << 16) |
 (signByte << 24) ;
 // value = 10^exponent * mantissa, pow() returns a double type
 return (int32_t) (pow(10, exponent) * mantissa);
} // FLOAT_TO_INT32
#endif
/**************************************************************************//**
 * @Function: handle_ble_event()
 * @Description: Handling Bluetooth events.
 * @Param sl_bt_msg_t *
 * @Return NULL
 *****************************************************************************/
void handle_ble_event(sl_bt_msg_t *evt)
{

  ble_data_struct_t *bleDataPtr = getBleDataPtr();
  switch (SL_BT_MSG_ID(evt->header))                       //Check for BLE Event
  {
    case sl_bt_evt_system_boot_id:                         //System Boot Started

      displayInit();
      #if DEVICE_IS_BLE_SERVER
      displayPrintf(DISPLAY_ROW_NAME, "Server");
      #else
      displayPrintf(DISPLAY_ROW_NAME, "Client");
      #endif
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A7");
      //Read Bluetooth address
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
#if DEVICE_IS_BLE_SERVER               //Server Mode
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

#else             //Client Mode
           sc =   sl_bt_scanner_set_mode(sl_bt_gap_1m_phy, SCAN_PASSIVE);
                if (sc != SL_STATUS_OK)
                  {
                    LOG_ERROR("sl_bt_scanner_set_mode() returned != 0 status=0x%04x", (unsigned int) sc);
                    break;
                  }
                sc =   sl_bt_scanner_set_timing(sl_bt_gap_1m_phy,SCAN_INTERVAL,SCAN_WINDOW);
                if (sc != SL_STATUS_OK)
                  {
                    LOG_ERROR("sl_bt_scanner_set_timing() returned != 0 status=0x%04x", (unsigned int) sc);
                    break;
                  }
                sc =   sl_bt_connection_set_default_parameters(DIS_MIN_INTERVAL,DIS_MAX_INTERVAL,SLAVE_LATENCY,SCAN_SUPERVISION_TIMEOUT,MIN_CE_LENGTH,MAX_CE_LENGTH);
                if (sc != SL_STATUS_OK)
                  {
                    LOG_ERROR("sl_bt_connection_set_default_parameters() returned != 0 status=0x%08x", (unsigned int) sc);
                    break;
                  }
                sc =   sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);
                if (sc != SL_STATUS_OK)
                  {
                    LOG_ERROR("sl_bt_scanner_start() returned != 0 status=0x%08x", (unsigned int) sc);
                    break;
                  }
      displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
#endif
      break;
    case sl_bt_evt_connection_opened_id:           //Event connection is Opened
      bleDataPtr->connection_handle=evt->data.evt_connection_opened.connection;
#if DEVICE_IS_BLE_SERVER    //Server Mode
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
#else
            bleDataPtr->flag_connection_open_close = true;

            displayPrintf(DISPLAY_ROW_BTADDR2, "%x:%x:%x:%x:%x:%x", ble_data.server_addr.addr[0], \
                               ble_data.server_addr.addr[1], ble_data.server_addr.addr[2], \
                               ble_data.server_addr.addr[3], ble_data.server_addr.addr[4], \
                               ble_data.server_addr.addr[5]);

#endif
      displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
      break;
    case sl_bt_evt_connection_closed_id:                   //Event Connection is closed
      bleDataPtr->flag_ok_to_send_htm_indications = false;
      bleDataPtr->flag_connection_open_close = false;
      bleDataPtr->flag_in_flight = false;
#if DEVICE_IS_BLE_SERVER        //Server Mode
      //Advertisement Start
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
            displayPrintf(DISPLAY_ROW_TEMPVALUE,"");
#else         //Client Mode
            sc = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);
            if(sc != SL_STATUS_OK) {
                LOG_ERROR("sl_bt_scanner_start() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
            }
            displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
#endif
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
      displayPrintf(DISPLAY_ROW_BTADDR2, "");
      break;
#if DEVICE_IS_BLE_SERVER
    case sl_bt_evt_connection_parameters_id://printing the connection values provided by the Client
      LOG_INFO("Interval = %d\n\r Latency = %d\n\r Timeout = %d\n\r",(int)((evt->data.evt_connection_parameters.interval)*1.25),
               (int)(evt->data.evt_connection_parameters.latency),
               (int)((evt->data.evt_connection_parameters.timeout)));
      break;
#endif
    case sl_bt_evt_system_soft_timer_id:
      //Indicates that a soft timer has lapsed.
      if(evt->data.evt_system_soft_timer.handle==0)
      {
          displayUpdate();
      }
      break;

#if DEVICE_IS_BLE_SERVER
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
       bleDataPtr->flag_ok_to_send_htm_indications = false;
       bleDataPtr->flag_in_flight=false;
       break;
#else
     case sl_bt_evt_scanner_scan_report_id://Connection established
       {
         int i = 0;
         uint8_t server_addr_check = 1;
         bd_addr server_address = SERVER_BT_ADDRESS;
         bleDataPtr->server_addr = server_address;
        while(i<=5)
         {
                 if(evt->data.evt_scanner_scan_report.address.addr[i] != server_address.addr[i])
                 {
                     server_addr_check = 0;
                     break;
                 }
                 i++;

         }
         if((server_addr_check == 1) &&(evt->data.evt_scanner_scan_report.packet_type == 0))
         {
           sc = sl_bt_scanner_stop();
           if (sc != SL_STATUS_OK)
             {
               LOG_ERROR("sl_bt_scanner_stop() returned != 0 status=0x%04x", (unsigned int) sc);
               break;
             }

           sc =  sl_bt_connection_open(evt->data.evt_scanner_scan_report.address,evt->data.evt_scanner_scan_report.address_type,sl_bt_gap_phy_1m,NULL);
           if (sc != SL_STATUS_OK)
           {
               LOG_ERROR("sl_bt_connection_open() returned != 0 status=0x%04x", (unsigned int) sc);
               break;
           }
         }
        }
       break; // handle open event
     case sl_bt_evt_gatt_service_id:
       bleDataPtr->thermometer_service_handle = evt->data.evt_gatt_service.service;
       break;
     case sl_bt_evt_gatt_characteristic_id:
       bleDataPtr->characteristic_handle = evt->data.evt_gatt_characteristic.characteristic;
 //      if( bleDataPtr -> gatt_procedure_completed == 1)
 //        {
 //          bleDataPtr -> gatt_procedure_completed = 0;
 //          sc = sl_bt_gatt_discover_characteristics_by_uuid(bleDataPtr->connection_handle,
 //                                                           bleDataPtr->thermometer_service_handle,
 //                                                           sizeof(thermo_char),
 //                                                           (const uint8_t*)thermo_char);
       break;
    case sl_bt_evt_gatt_characteristic_value_id:
      // Indicates that the value of remote GATT server was received
      sc = sl_bt_gatt_send_characteristic_confirmation(bleDataPtr->connection_handle); //Connection handle
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
          break;
      }
      else
      {
      //Save the value which you have received from server
          displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
          bleDataPtr->temp_value = &(evt->data.evt_gatt_characteristic_value.value.data[0]);
          temperature_in_c = FLOAT_TO_INT32((bleDataPtr->temp_value));
          displayPrintf(DISPLAY_ROW_TEMPVALUE, "temp=%d",temperature_in_c);
      }
      break;
#endif
  }
}
#if DEVICE_IS_BLE_SERVER
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
#endif
