/*******************************************************************************
 * ble.c
 * Date:        25-02-2022
 * Author:      Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
                Rajesh
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
#include "src/scheduler.h"
#include "src/gpio.h"
#include "src/i2c.h"
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
#define LATENCY        3
#define TIMEOUT                  800  //1000ms
#define SCAN_PASSIVE                  0
#define CONFIRM_BONDING 1
#define SM_CONFIGURATION_FLAG 0x0F
#define QUEUE_DEPTH      (16)
#define TWOFIFTY_MILLISECOND 8192
#define QUEUE_HANDLE 1
#define CONTINUOUS_TIMER 0
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
#define SM_CONFIGURATION_FLAG 0x0F
#endif
int32_t temperature_in_c=0;
ble_data_struct_t ble_data;
sl_status_t sc;
uint32_t         wptr = 0;              // write pointer
uint32_t         rptr = 0;              // read pointer
uint32_t length = 0; //Total elements in the array
uint8_t button_state = 0x00;
bool button_gatt_disable = 0;
ble_data_struct_t* getBleDataPtr() {
  return (&ble_data);
} // getBleDataPtr()
#if DEVICE_IS_BLE_SERVER
static uint32_t nextPtr(uint32_t);
extern volatile uint8_t pir_count ;
typedef struct {

  uint16_t  charHandle;
  uint32_t bufferLength;
  uint8_t buffer[5];

} queue_struct_t;

queue_struct_t characteristics[QUEUE_DEPTH];
#endif
/****************************************/
// -----------------------------------------------
// Private function, original from Dan Walkes. I fixed a sign extension bug.
// We'll need this for Client A7 assignment to convert health thermometer
// indications back to an integer. Convert IEEE-11073 32-bit float to signed integer.
// -----------------------------------------------
#if !(DEVICE_IS_BLE_SERVER)
uint16_t lux_value =0;
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
#if DEVICE_IS_BLE_SERVER
static uint32_t nextPtr(uint32_t ptr)
{
  if (ptr+1 >= QUEUE_DEPTH)
    return 0;     // wrap back to 0
  else
    return ptr+1; // advance

} // nextPtr()
void enque_characteristics(uint16_t charHandles,uint32_t bufferLength,uint8_t* buffer)
{
  if(length <QUEUE_DEPTH)
    {

      //Store the data into queue
      characteristics[wptr].charHandle = charHandles;
      characteristics[wptr].bufferLength = bufferLength;
      for(uint32_t i =0;i<bufferLength;i++)
        {
          characteristics[wptr].buffer[i] = buffer[i];
        }

      wptr = nextPtr(wptr);
      length++;
      //LOG_INFO("length =%dC\n\r",length);
    }
}
void dequeue_characteristics()
{
  ble_data_struct_t *bleDataPtr = getBleDataPtr();
  uint16_t  charHandles;
  uint32_t bufferLength;
  uint8_t buffer[5];

  if((length >= 1))
    {
      charHandles = characteristics[rptr].charHandle;
      bufferLength = characteristics[rptr].bufferLength;
      for(uint32_t i =0;i<bufferLength;i++)
        {
          buffer[i] = characteristics[rptr].buffer[i];
        }
      rptr = nextPtr(rptr);
      length--;

      sc = sl_bt_gatt_server_send_indication(bleDataPtr->ble_connection_handle,charHandles,bufferLength,buffer);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x", (unsigned int) sc);
        }
      else
        {
          bleDataPtr->flag_in_flight = true;
        }
      //LOG_INFO("length =%dC\n\r",length);
    }
  else
    {

    }
}
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
      bleDataPtr->flag_connection_open_close = false;
      bleDataPtr->flag_ok_to_send_ambient_light_indications = false;
      bleDataPtr->flag_ok_to_send_PIR_indications = false;
      bleDataPtr->flag_in_flight=false;
      displayInit();
      //  bleDataPtr->bonding_complete = false;
      //  bleDataPtr->confirm_pass_key = false;
#if DEVICE_IS_BLE_SERVER
      displayPrintf(DISPLAY_ROW_NAME, "Server");
#else
      displayPrintf(DISPLAY_ROW_NAME, "Client");
#endif
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "PROJECT");
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
     //  sl_bt_sm_configure(SM_CONFIGURATION_FLAG, sm_io_capability_displayyesno);

       //           sc= sl_bt_sm_delete_bondings();
         //               if (sc != SL_STATUS_OK)
           //             {
             //                    LOG_ERROR("sl_bt_sm_delete_bondings() returned != 0 status=0x%04x", (unsigned int) sc);
               //                  break;
                 //       }
                     //   else
                   //       {
                       //     bleDataPtr->bonding_complete = false;
                         // }
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
//      sc= sl_bt_sm_delete_bondings();
//      sl_bt_sm_configure(SM_CONFIGURATION_FLAG, sm_io_capability_displayyesno);
      displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
      displayPrintf(DISPLAY_ROW_9, "");
#endif
      break;
    case sl_bt_evt_connection_opened_id:           //Event connection is Opened
      //
      #if DEVICE_IS_BLE_SERVER    //Server Mode
      bleDataPtr->ble_connection_handle=evt->data.evt_connection_opened.connection;
      sc= sl_bt_advertiser_stop(bleDataPtr->advertisingSetHandle);

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
           displayPrintf(DISPLAY_ROW_9, "" );
        }

      /*   sl_status_t   timer_response;//Trigger dequeue() every 250ms
      timer_response = sl_bt_system_set_soft_timer(TWOFIFTY_MILLISECOND,QUEUE_HANDLE,CONTINUOUS_TIMER);
      if (timer_response != SL_STATUS_OK)
      {
         LOG_ERROR("Error in timer response");
      }
      sc= sl_bt_sm_delete_bondings();
      if (sc != SL_STATUS_OK)
      {
                    LOG_ERROR("sl_bt_sm_delete_bondings() returned != 0 status=0x%04x", (unsigned int) sc);
                    break;
      }*/
      #else
      bleDataPtr->connection_handle=evt->data.evt_connection_opened.connection;
          bleDataPtr->flag_connection_open_close = true;

      displayPrintf(DISPLAY_ROW_BTADDR2, "%x:%x:%x:%x:%x:%x", ble_data.server_addr.addr[0], \
                    ble_data.server_addr.addr[1], ble_data.server_addr.addr[2], \
                    ble_data.server_addr.addr[3], ble_data.server_addr.addr[4], \
                    ble_data.server_addr.addr[5]);

      #endif
       displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
      break;
    //  case sl_bt_evt_sm_confirm_passkey_id:
      //{
      //uint32_t passkey = evt->data.evt_sm_confirm_passkey.passkey;
      //bleDataPtr->confirm_pass_key = true;
      //displayPrintf(DISPLAY_ROW_PASSKEY,"%d",passkey);
      //displayPrintf(DISPLAY_ROW_ACTION,"Confirm with PB0");
      //sl_bt_sm_passkey_confirm(bleDataPtr->ble_connection_handle,0x01);
      //}
      //break;
      //case sl_bt_evt_sm_bonded_id:
       // bleDataPtr->bonding_complete = true;
        //displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded" );
//#if DEVICE_IS_BLE_SERVER
//        displayPrintf(DISPLAY_ROW_9, "Button Released");
//#endif
//        displayPrintf(DISPLAY_ROW_PASSKEY,"");
//        displayPrintf(DISPLAY_ROW_ACTION,"");
//#if !(DEVICE_IS_BLE_SERVER)
//        displayPrintf(DISPLAY_ROW_9,"Button Released");
//#endif
//        break;
/*#if DEVICE_IS_BLE_SERVER
  //  case sl_bt_evt_sm_confirm_bonding_id:

      if(evt->data.evt_sm_confirm_bonding.bonding_handle == SL_BT_INVALID_BONDING_HANDLE)
      {
          sc = sl_bt_sm_bonding_confirm(bleDataPtr->ble_connection_handle,CONFIRM_BONDING);
          if (sc != SL_STATUS_OK)
          {
                   LOG_ERROR("sl_bt_sm_bonding_confirm() returned != 0 status=0x%04x", (unsigned int) sc);
                   break;
          }
      }
      break;
#endif
  case sl_bt_evt_sm_bonding_failed_id:
      bleDataPtr->bonding_complete = false;
      LOG_ERROR("Bonding Failed");
      break;*/
    case sl_bt_evt_connection_closed_id:                   //Event Connection is closed
      //   bleDataPtr->bonding_complete = false;
      //  bleDataPtr->confirm_pass_key = false;
      bleDataPtr->flag_connection_open_close = false;
      bleDataPtr->flag_in_flight=false;

      /*  sc= sl_bt_sm_delete_bondings();
        if (sc != SL_STATUS_OK)
         {
                      LOG_ERROR("sl_bt_sm_delete_bondings() returned != 0 status=0x%04x", (unsigned int) sc);
                      break;
        }*/

#if DEVICE_IS_BLE_SERVER        //Server Mode
      //Advertisement Start
  // case sl_bt_evt_connection_closed_id:
      sc=sl_bt_advertiser_start(bleDataPtr->advertisingSetHandle,sl_bt_advertiser_general_discoverable,sl_bt_advertiser_connectable_scannable);
      bleDataPtr->flag_ok_to_send_ambient_light_indications = false;
      bleDataPtr->flag_ok_to_send_PIR_indications = false;
      if (sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x", (unsigned int) sc);
      }
     else
      {
          displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising" );
      }
      gpioLed0SetOff();
      gpioLed1SetOff();
      //break;
//        sc= sl_bt_sm_delete_bondings();
//         if (sc != SL_STATUS_OK)
//         {
//                LOG_ERROR("sl_bt_sm_delete_bondings() returned != 0 status=0x%04x", (unsigned int) sc);
//              break;
//        }
//       //handle close events
//         displayPrintf(DISPLAY_ROW_TEMPVALUE,"");
//         displayPrintf(DISPLAY_ROW_ACTION,"");
//         displayPrintf(DISPLAY_ROW_9,"");
#else         //Client Mode
      sc = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_scanner_start() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }
      displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
      displayPrintf(DISPLAY_ROW_9, "");
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
      /*#if DEVICE_IS_BLE_SERVER
      if(evt->data.evt_system_soft_timer.handle==1)
      {
          ble_data_struct_t *bleDataPtr = getBleDataPtr();

         if((bleDataPtr->flag_in_flight == false)&&(bleDataPtr->flag_ok_to_send_pb0_indications == true||bleDataPtr->flag_ok_to_send_htm_indications==true))
           {
             dequeue_characteristics();
           }
      }*/
      //#endif
      //  break;
#if DEVICE_IS_BLE_SERVER
     case sl_bt_evt_gatt_server_characteristic_status_id://Verify if data is updated in GATT server and ack received from Client
       if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_disable)
         {
           if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_lux_measurement)
           {
               bleDataPtr->flag_ok_to_send_ambient_light_indications = false;
               gpioLed0SetOff();
               displayPrintf(DISPLAY_ROW_TEMPVALUE,"");
           }
           if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_IR_Detection)
           {
               bleDataPtr->flag_ok_to_send_PIR_indications = false;
               gpioLed1SetOff();
           }
         }
       else if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_indication)
         {
           if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_lux_measurement)
             {
               bleDataPtr->flag_ok_to_send_ambient_light_indications = true;
               gpioLed0SetOn();
             }
           if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_IR_Detection)
             {
               bleDataPtr->flag_ok_to_send_PIR_indications = true;
               gpioLed1SetOn();
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
      bleDataPtr->flag_ok_to_send_PIR_indications = false;
      bleDataPtr->flag_ok_to_send_ambient_light_indications = false;
      // bleDataPtr->bonding_complete = false;
      // bleDataPtr->confirm_pass_key = false;
       gpioLed0SetOff();
       gpioLed1SetOff();
      break;
    case sl_bt_evt_system_external_signal_id:
      if(evt->data.evt_system_external_signal.extsignals == evtgpiopir1intset ||evt->data.evt_system_external_signal.extsignals == evtgpiopir2intset)
      {
          if(bleDataPtr->flag_in_flight == false)
            {
              PIR_measurement();
            }
//          if(bleDataPtr->bonding_complete == true)
//            {
//              ble_send_pb0_indication(0x00);
//              displayPrintf(DISPLAY_ROW_9, "Button Released");
//            }
      }
//      else if(evt->data.evt_system_external_signal.extsignals == evtgpiopb0intclear)
//      {
//          if(bleDataPtr->confirm_pass_key == true)
//          {
//              sl_bt_sm_passkey_confirm(bleDataPtr->ble_connection_handle,0x01);
//          }
//          if(bleDataPtr->bonding_complete == true)
//          {
//              ble_send_pb0_indication(0x01);
//              displayPrintf(DISPLAY_ROW_9, "Button Pressed");
//          }
//      }
      else
      {

      }
      break;
#else
//    case sl_bt_evt_system_external_signal_id:
//      if(evt->data.evt_system_external_signal.extsignals == evtgpiopb0intclear)//press
//        {
//          //4 bits are used to obtain the PB0-press; PB0-release; PB1-Press;PB1-Release for indication
//          //Bit number                      0      ;  1         ;     2    ;    3
//          if(bleDataPtr->confirm_pass_key == true)
//            {
//              sl_bt_sm_passkey_confirm(bleDataPtr->connection_handle,0x01);
//              bleDataPtr->confirm_pass_key = false;
//            }
//          button_state |= (1<<0);
//        }
//      else if(evt->data.evt_system_external_signal.extsignals == evtgpiopb0intset)//release
//        {
//          button_state |= (1<<1);
//          //Indicate logic
//          if(button_state == 0x03)
//            {
//              button_state = 0x00;
//            }
//          else if(button_state == 0x0F)
//            {
//              if(button_gatt_disable == 0)//Flag is used for toggling operation
//                {
//                  sc = sl_bt_gatt_set_characteristic_notification(bleDataPtr->connection_handle,
//                                                                  gattdb_button_state,
//                                                                  sl_bt_gatt_disable);
//                  button_gatt_disable = 1;
//                }
//              else
//                {
//                  sc = sl_bt_gatt_set_characteristic_notification(bleDataPtr->connection_handle,
//                                                                  gattdb_button_state,
//                                                                  sl_bt_gatt_indication);
//                  button_gatt_disable = 0;
//                }
//              if(sc != SL_STATUS_OK)
//                {
//                  LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
//                }
//              button_state = 0x00;
//            }
//          else
//            {
//              button_state = 0x00;
//            }
//        }
//      else
//        {
//
//        }
//      if(evt->data.evt_system_external_signal.extsignals == evtgpiopb1intset)//release
//        {
//          if(button_state != 0x05)
//            {
//              button_state = 0x00;
//            }
//          else
//            {
//              button_state |= (1<<3);
//            }
//
//        }
//      else if(evt->data.evt_system_external_signal.extsignals == evtgpiopb1intclear)//press
//        {
//          //Check for PB0 also
//          if((bleDataPtr->flag_in_flight == false) && (button_state!= 0x01))
//            {
//              sc = sl_bt_gatt_read_characteristic_value(bleDataPtr->connection_handle,gattdb_button_state);//Obtain read information
//              bleDataPtr->flag_in_flight = true;
//              if (sc != SL_STATUS_OK)
//                {
//                  LOG_ERROR("sl_bt_gatt_read_characteristic_value() returned != 0 status=0x%04x", (unsigned int) sc);
//                  break;
//                }
//            }
//          else
//            {
//              button_state |= (1<<2);
//            }
//        }
//      else
//        {
//
//        }
//      break;
//    case sl_bt_evt_gatt_procedure_completed_id:
//      bleDataPtr->flag_in_flight = false;
//      if(evt->data.evt_gatt_procedure_completed.result == SL_STATUS_BT_ATT_INSUFFICIENT_ENCRYPTION)
//        {
//          sl_bt_sm_increase_security(bleDataPtr->connection_handle);//Increase security for push button
//        }
//      break;
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
      bleDataPtr->service_handle = evt->data.evt_gatt_service.service;
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

      // Indicates that GATT server transmitted data
//      if(evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_read_response)
//        {
//          bleDataPtr->button_value = (evt->data.evt_gatt_characteristic_value.value.data[0]);
//          if( bleDataPtr->button_value == 0x01)
//            {
//              displayPrintf(DISPLAY_ROW_9, "Button Pressed");
//            }
//          else
//            {
//
//              displayPrintf(DISPLAY_ROW_9, "Button Released");
//            }
//          bleDataPtr->flag_in_flight = false;
//        }
          sc = sl_bt_gatt_send_characteristic_confirmation(bleDataPtr->connection_handle);
          if(sc != SL_STATUS_OK)
            {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              break;
            }
          if(evt->data.evt_gatt_characteristic_value.characteristic == gattdb_lux_measurement)
            {
              //Save the value which you have received from server
//              if(bleDataPtr->bonding_complete != true)
//                {
                  displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
                //}
              bleDataPtr->lux_value = (evt->data.evt_gatt_characteristic_value.value.data[2]);
              lux_value =    bleDataPtr->lux_value | ((evt->data.evt_gatt_characteristic_value.value.data[1])<<8);
              //temperature_in_c = FLOAT_TO_INT32((bleDataPtr->temp_value));
              displayPrintf(DISPLAY_ROW_TEMPVALUE, "Lux=%d",lux_value);
            }
          if(evt->data.evt_gatt_characteristic_value.characteristic == gattdb_IR_Detection)
            {
              bleDataPtr->pir_value = (evt->data.evt_gatt_characteristic_value.value.data[1]);

                  displayPrintf(DISPLAY_ROW_9,"people_count=%d",bleDataPtr->pir_value);
            }
          else
            {

            }

      break;
#endif
  }
}
#if DEVICE_IS_BLE_SERVER
//Author Rajesh
void ambient_light_measurement(uint16_t read_lux)
{
  uint8_t ALS_buffer[3];
  ALS_buffer[0] = 0x00;
  ALS_buffer[1] = (read_lux>>8)&0xFF;
  ALS_buffer[2] = read_lux&0xFF;
  ble_data_struct_t *bleDataPtr = getBleDataPtr();
  sc=sl_bt_gatt_server_write_attribute_value(gattdb_lux_measurement,0,2,&ALS_buffer[1]);
  //LOG_INFO("ALS Value: %d\n\r",*ALS_value);
  //displayPrintf("ALS Value: %d\n\r",*ALS_value);
  displayPrintf(DISPLAY_ROW_TEMPVALUE,"ALS Value: %d",read_lux);
  if(bleDataPtr->flag_connection_open_close == true  && bleDataPtr->flag_ok_to_send_ambient_light_indications == true)
    {
      if(bleDataPtr->flag_in_flight == false)
        {
          sc = sl_bt_gatt_server_send_indication(bleDataPtr->ble_connection_handle,gattdb_lux_measurement,3,ALS_buffer);
          if (sc != SL_STATUS_OK)
            {
              LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x", (unsigned int) sc);
            }
          else
            {
              bleDataPtr->flag_in_flight = true;
            }
        }
      // displayPrintf(DISPLAY_ROW_10,"ALS Value: %d\n\r",*ALS_value);
      //LOG_INFO("ALS Value: %d\n\r",*ALS_buffer);
    }
}
//Author rajesh
void PIR_measurement()
{
  uint8_t PIR_buffer[2];
  PIR_buffer[0] = 0x00;
  PIR_buffer[1] = pir_count;
  ble_data_struct_t *bleDataPtr = getBleDataPtr();
  sc=sl_bt_gatt_server_write_attribute_value(gattdb_IR_Detection,0,1,&PIR_buffer[1]);
  //LOG_INFO("ALS Value: %d\n\r",*ALS_value);
 // LOG_INFO("PIR Value: %d\n\r",pir_count);
  displayPrintf(DISPLAY_ROW_10,"PIR Value: %d",pir_count);
  if(bleDataPtr->flag_connection_open_close == true  && bleDataPtr->flag_ok_to_send_PIR_indications == true)
    {
      if(bleDataPtr->flag_in_flight == false)
        {
          sc = sl_bt_gatt_server_send_indication(bleDataPtr->ble_connection_handle,gattdb_IR_Detection,2,PIR_buffer);
          if (sc != SL_STATUS_OK)
            {
              LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x", (unsigned int) sc);
            }
          else
            {
              bleDataPtr->flag_in_flight = true;
            }
        }
      //LOG_INFO("PIR Value: %d\n\r",pir_count);
    }
 // return pir_count;
}

#endif
