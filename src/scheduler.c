/*******************************************************************************
 * scheduler.c
 * Date:        10-02-2022
 * Author:      Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
 *              Rajesh Srirangam
 * Description: This file has scheduling related information
 *
 *
 *
 ******************************************************************************/

#include "scheduler.h"
#include "em_core.h"
#include "i2c.h"
#include "em_letimer.h"
#include "gpio.h"
#include "src/ble.h"
#include "src/timer.h"
#include "gatt_db.h"
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"
#include "src/ble_device_type.h"
// scheduler routine to set a scheduler event

//#define POWER_UP_TIME 80000 // 80 milli sec in microseconds
#define AMBIENT_READ_WAIT_TIME 100000 //100 milli sec in microseconds
//#define TEMP_READ_WAIT_TIME 10800 // 10.8 milli sec microseconds

#define INCLUDE_LOG_DEBUG 1
#include "log.h"

uint16_t read_data = 0; // temperature data
uint16_t read_lux_data = 0;
uint16_t write_register = 2048;
I2C_TransferReturn_TypeDef transferStatus; // Status of data transfer

//States of I2C machines
enum I2C_states
{
  i2c_idle = 1,
  i2c_setup_time,
  i2c_write,
  i2c_write_wait,
  i2c_read
};

uint16_t myEvent=0;//Events that are triggered

/****************************************************************
 *@Function void schedulerSetEventTemperaturemeasurement()
 *@Description Called when COMP0 interrupt is triggered. //UF flag
 *@Param NULL
 *@Return NULL
 ****************************************************************/
void schedulerSetEventAmbiencemeasurement()
{

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  // LOG_INFO("Entering schedulerSetEventTemperaturemeasurement\n\r");
 // sl_bt_external_signal(evtReadTemperature);
   myEvent |=evtReadAmbientsensor;
  CORE_EXIT_CRITICAL();
} // schedulerSetEventXXX()

/****************************************************************
 *@Function void schedulerSetEventcomp1set()
 *@Description Triggers when COMP1 interrupt is triggered
 *@Param NULL
 *@Return NULL
 ****************************************************************/
void schedulerSetEventcomp1set()
{

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
//  sl_bt_external_signal(evti2ccomp1setcomplete);
 myEvent |=evti2ccomp1setcomplete;
  //LOG_INFO("Entering schedulerSetEventcomp1set\n\r");
  //LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
  CORE_EXIT_CRITICAL();
} // schedulerSetEventXXX()


/********************************************************************
 *@Function void schedulerSetEventi2cTransferDone()
 *@Description Triggers when I2C interrupt transfer interrupt is set
 *@Param NULL
 *@Return NULL
 ********************************************************************/
void schedulerSetEventi2cTransferDone()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
 // sl_bt_external_signal(evti2ctransfercomplete);
  myEvent |=evti2ctransfercomplete;
  //   LOG_INFO("Entering schedulerSetEventi2cTransferDone\n\r");
  //LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
  CORE_EXIT_CRITICAL();
}

/********************************************************************
 *@Function void schedulerSetEventGPIOPB0()
 *@Description Triggers when PB0 pin is set
 *@Param NULL
 *@Return NULL
 ********************************************************************/
void schedulerSetEventGPIOPB0set()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  //gpioLed0SetOn();
  sl_bt_external_signal(evtgpiopb0intset);
  CORE_EXIT_CRITICAL();
}

/********************************************************************
 *@Function void schedulerSetEventGPIOPB0()
 *@Description Triggers when PB0 pin is set
 *@Param NULL
 *@Return NULL
 ********************************************************************/
void schedulerSetEventGPIOPB0clear()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  //gpioLed0SetOff();
  sl_bt_external_signal(evtgpiopb0intclear);
  CORE_EXIT_CRITICAL();
}

/********************************************************************
 *@Function void schedulerSetEventGPIOPB0()
 *@Description Triggers when PB0 pin is set
 *@Param NULL
 *@Return NULL
 ********************************************************************/
void schedulerSetEventGPIOPB1set()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtgpiopb1intset);
  CORE_EXIT_CRITICAL();
}
/********************************************************************
 *@Function void schedulerSetEventPIRtriggeredset()
 *@Description Triggers when PIR sensor triggered
 *@Param NULL
 *@Return NULL
 ********************************************************************/
void schedulerSetEventPIRtriggeredset()
{
  //Write code here
  //LOG_INFO("Motion detected!\n");
  gpioLed0SetOn();
}
/********************************************************************
 *@Function void schedulerSetEventGPIOPB0()
 *@Description Triggers when PB0 pin is set
 *@Param NULL
 *@Return NULL
 ********************************************************************/
void schedulerSetEventGPIOPB1clear()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtgpiopb1intclear);
  CORE_EXIT_CRITICAL();
}
/*******************************************************
 *@Function void getNextEvent()
 *@Description Obtain Next Event which is triggered
 *@Param NULL
 *@Return NULL
 *******************************************************/
uint32_t getNextEvent()
{
  uint32_t theEvent = 0;

      //IRQ Disabled
      CORE_DECLARE_IRQ_STATE;
      CORE_ENTER_CRITICAL();
      if(myEvent & evtReadAmbientsensor)
      {
        theEvent = evtReadAmbientsensor; // 1 event to return to the caller
        myEvent &=~evtReadAmbientsensor;
      }
      else if(myEvent & evti2ccomp1setcomplete)
      {
        theEvent = evti2ccomp1setcomplete; // 1 event to return to the caller
        myEvent &=~evti2ccomp1setcomplete;
      }
      else if(myEvent & evti2ctransfercomplete)
      {
        theEvent = evti2ctransfercomplete; // 1 event to return to the caller
        myEvent &=~evti2ctransfercomplete;
      }
      CORE_EXIT_CRITICAL();

  return theEvent;
}
//#if DEVICE_IS_BLE_SERVER
/*******************************************************
 *@Function void temperature_state_machine()
 *@Description State machine to communicate via i2c
 *@Param NULL
 *@Return NULL
 *******************************************************/
//Ambient state machine
//Author Rajesh and Sriraj
void ambient_light_state_machine(int event)
{
 // uint32_t eventValue = evt->data.evt_system_external_signal.extsignals;
  static uint32_t current_state = i2c_idle;
 // if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_system_external_signal_id)
 //   {
      switch(current_state)
      {
        case i2c_idle:
          if(event == evtReadAmbientsensor)
            {
              I2C_init();//Initialize I2C
            //  LOG_INFO("State 1\n\r");
              i2c_veml6030_write_cmd();
              //timerwaitus_irq(AMBIENT_READ_WAIT_TIME);
              current_state = i2c_write_wait;
            }
          break;
        /*case i2c_setup_time:
          if(event && evti2ccomp1setcomplete)
            {
              //LOG_INFO("State 2\n\r");

              current_state = i2c_write;
            }
          break;*/
        case i2c_write:
          //LOG_INFO("cw\n\r");
          {

              //sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);//Add sl power management
              //timerwaitus_irq(AMBIENT_READ_WAIT_TIME);
            //  LOG_INFO("State 4\n\r");
              //NVIC_DisableIRQ(I2C0_IRQn);
              //sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
              //comp1 update value
             // timerwaitus_irq(AMBIENT_READ_WAIT_TIME);
              current_state = i2c_write_wait;
            }
          break;
        case i2c_write_wait://Wait for computation to complete
          if(event == evti2ctransfercomplete)
            {
              NVIC_DisableIRQ(I2C0_IRQn);
              i2c_veml6030_write_read_cmd(&read_lux_data);
              current_state = i2c_read;
            }
//          else
//            {
//              current_state = i2c_write;
//            }
          break;
        case i2c_read:
          if(event == evti2ctransfercomplete)
            {
              //i2c_veml6030_write_read_cmd(0x04,&read_lux_data);
            //  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
              //LOG_INFO("State 4\n\r");
              NVIC_DisableIRQ(I2C0_IRQn);
             // sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
              LOG_INFO("LUX VALUE is=%d C\n\r",read_lux_data);
              // ble_send_temp(temperature);
              //i2c_deinitialize();
              current_state = i2c_idle;
            }
          break;
        default:
          break;
      }
    }
//}
/*#else

/*void discovery_state_machine(sl_bt_msg_t *evt)
{
  sl_status_t sc=0;
  static discovery_state current_state = PRIMARY_CHAR_UUID;
  ble_data_struct_t *bleDataPtr = getBleDataPtr();
  if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id)
    {
      current_state = PRIMARY_CHAR_UUID;
    }
  switch(current_state)
  {
    //Discover primary services by UUID
    case PRIMARY_CHAR_UUID:
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_opened_id)
        {
          sc = sl_bt_gatt_discover_primary_services_by_uuid(bleDataPtr->connection_handle,
                                                            sizeof(thermo_service),
                                                            (const uint8_t*)thermo_service);
          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() 1 returned != 0 status=0x%04x\n\r", (unsigned int)sc);

          }
          else
            {
              current_state = DISCOVER_CHAR_UUID;
            }
        }
      break;
      //Discover char by UUID
    case DISCOVER_CHAR_UUID:
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {

          sc = sl_bt_gatt_discover_characteristics_by_uuid(bleDataPtr->connection_handle,
                                                           bleDataPtr->service_handle,
                                                           sizeof(thermo_char),
                                                           (const uint8_t*)thermo_char);
          if(sc != SL_STATUS_OK)
            {
              LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() 1 returned != 0 status=0x%04x\n\r", (unsigned int)sc);
            }
          else
            {
              current_state =SET_CHAR_NOTIFY;
            }
      }
      break;
      //Get characteristic notification
    case SET_CHAR_NOTIFY:
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {

          sc = sl_bt_gatt_set_characteristic_notification(bleDataPtr->connection_handle,
                                                          gattdb_temperature_measurement,
                                                          sl_bt_gatt_indication);
          if(sc != SL_STATUS_OK)
            {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
            }
          else
            {
              current_state = PRIMARY_BUTTON_CHAR_UUID;
            }
      }
      break;
    case PRIMARY_BUTTON_CHAR_UUID:
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {
          sc = sl_bt_gatt_discover_primary_services_by_uuid(bleDataPtr->connection_handle,
                                                            sizeof(push_button_service),
                                                            (const uint8_t*)push_button_service);
          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() 1 returned != 0 status=0x%04x\n\r", (unsigned int)sc);
          }
          else
            {
              current_state =  DISCOVER_BUTTON_CHAR_UUID;
            }
      }
      break;
    case DISCOVER_BUTTON_CHAR_UUID:
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {

          sc = sl_bt_gatt_discover_characteristics_by_uuid(bleDataPtr->connection_handle,
                                                           bleDataPtr->service_handle,
                                                           sizeof(push_button_char),
                                                           (const uint8_t*)push_button_char);
          if(sc != SL_STATUS_OK)
            {
              LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() 1 returned != 0 status=0x%04x\n\r", (unsigned int)sc);
            }
          else
            {
              current_state =SET_CHAR_BUTTON_NOTIFY;
            }
      }
      break;
    case SET_CHAR_BUTTON_NOTIFY:
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {

          sc = sl_bt_gatt_set_characteristic_notification(bleDataPtr->connection_handle,
                                                          gattdb_button_state,
                                                          sl_bt_gatt_indication);
          if(sc != SL_STATUS_OK)
            {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
            }
          else
            {
              current_state = DISCOVERY_COMPLETE;
            }
      }
      break;
      //Discovery complete event
    case DISCOVERY_COMPLETE:
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {

          current_state =CLOSE_CONNECTION;
        }
      else
        {
          current_state = PRIMARY_CHAR_UUID;
        }
      break;
      //Closing the connection
    case CLOSE_CONNECTION:
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id)
        {
          current_state = PRIMARY_CHAR_UUID;
        }
      break;
  }
}
#endif */
