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
#define INCLUDE_LOG_DEBUG 0
#include "src/log.h"
#include "src/ble_device_type.h"
#include "src/irq.h"
#include <stdbool.h>
// scheduler routine to set a scheduler event

//#define POWER_UP_TIME 80000 // 80 milli sec in microseconds
#define AMBIENT_READ_WAIT_TIME 100000 //100 milli sec in microseconds
//#define TEMP_READ_WAIT_TIME 10800 // 10.8 milli sec microseconds


uint16_t read_data = 0; // temperature data
uint16_t read_lux_data = 0;
uint16_t write_register = 2048;
I2C_TransferReturn_TypeDef transferStatus; // Status of data transfer
int count=0;
bool state_t=false;
bool entry_flag = 0;
volatile bool pir_1=false;
volatile bool pir_2=false;
volatile uint8_t pir_count = 0;
//uint8_t pir_leave_count = 0;
//States of I2C machines
enum I2C_states
{
  i2c_init = 1,
  i2c_read,
  i2c_calculate
};

uint16_t myEvent=0;//Events that are triggered

/****************************************************************
 *@Function void schedulerSetEventTemperaturemeasurement()
 *@Description Called when COMP0 interrupt is triggered. //UF flag
 *@Param NULL
 *@Return NULL
 ****************************************************************/

/****************************************************************
 *@Function void schedulerSetEventTemperaturemeasurement()
 *@Description Called when COMP0 interrupt is triggered. //UF flag
 *@Param NULL
 *@Return NULL
 ****************************************************************/
void schedulerSetCountPIR_1_detect()
{

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  // LOG_INFO("Entering schedulerSetEventTemperaturemeasurement\n\r");
  // sl_bt_external_signal(evtReadTemperature);

  pir_1 = true;
  sl_bt_external_signal(evtgpiopir1intset);
  LOG_INFO("schedulerSetCountPIR_1_detect:pir_2:%d\n\r",pir_2);
  LOG_INFO("schedulerSetCountPIR_1_detect:pir_1:%d\n\r",pir_1);
  if (pir_1 & pir_2) {
      pir_1 = false;
      pir_2 = false;
      if (pir_count > 0) {
        pir_count--;
        LOG_INFO("schedulerSetCountPIR_1_detect: pir_count: %d\n\r", pir_count);
      } else {
        LOG_INFO("schedulerSetCountPIR_1_detect: pir_count already zero\n\r");
      }
    }  else if (pir_2) {
      pir_2 = false;
      LOG_INFO("schedulerSetCountPIR_1_detect: pir_2_triggered\n\r");
    }
  CORE_EXIT_CRITICAL();
} // schedulerSetEventXXX()

/****************************************************************
 *@Function void schedulerSetEventTemperaturemeasurement()
 *@Description Called when COMP0 interrupt is triggered. //UF flag
 *@Param NULL
 *@Return NULL
 ****************************************************************/
void schedulerSetCountPIR_2_detect()
{

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  // LOG_INFO("Entering schedulerSetEventTemperaturemeasurement\n\r");
  // sl_bt_external_signal(evtReadTemperature);

  pir_2 = true;
  sl_bt_external_signal(evtgpiopir2intset);
  LOG_INFO("schedulerSetCountPIR_2_detect:pir_2:%d\n\r",pir_2);
  LOG_INFO("schedulerSetCountPIR_2_detect:pir_1:%d\n\r",pir_1);
  if (pir_1 & pir_2)
    {
      pir_1 = false;
      pir_2 = false;
    pir_count++;
    }
  else if(pir_1) {
      pir_1 = false;
       LOG_INFO("schedulerSetCountPIR_2_detect: pir_1_triggered\n\r");
     }
  else if(pir_2) {
         pir_2 = false;
       LOG_INFO("schedulerSetCountPIR_2_detect: pir_2_triggered\n\r");
     }
  else
    {
      //
   }

  CORE_EXIT_CRITICAL();
} // schedulerSetEventXXX()
void schedulerSetEventAmbiencemeasurement()
{

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  // LOG_INFO("Entering schedulerSetEventTemperaturemeasurement\n\r");
  // sl_bt_external_signal(evtReadTemperature);
  sl_bt_external_signal(evtReadAmbientsensor);
  //myEvent |=evtReadAmbientsensor;
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
 // myEvent |=evti2ccomp1setcomplete;
  sl_bt_external_signal(evti2ccomp1setcomplete);

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
  //myEvent |=evti2ctransfercomplete;
  sl_bt_external_signal(evti2ctransfercomplete);
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

#if DEVICE_IS_BLE_SERVER
/*******************************************************
 *@Function void temperature_state_machine()
 *@Description State machine to communicate via i2c
 *@Param NULL
 *@Return NULL
 *******************************************************/
//Ambient state machine
//Author Rajesh and Sriraj
void ambient_light_state_machine(sl_bt_msg_t *evt)
{

  // uint32_t eventValue = evt->data.evt_system_external_signal.extsignals;
  static uint32_t current_state = i2c_init;

//  if(pir_count > 0)
//    {
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_system_external_signal_id)
      {
      switch(current_state)
      {
        case i2c_init:
          if(evt->data.evt_system_external_signal.extsignals  == evtReadAmbientsensor)
            {
              I2C_init();//Initialize I2C
              //  LOG_INFO("State 1\n\r");
              sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
              i2c_veml6030_write_cmd();
              //timerwaitus_irq(AMBIENT_READ_WAIT_TIME);
              current_state = i2c_read;
            }
          break;
        case i2c_read://Wait for computation to complete
          if(evt->data.evt_system_external_signal.extsignals  == evti2ctransfercomplete)
            {
              NVIC_DisableIRQ(I2C0_IRQn);
              //sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
              i2c_veml6030_write_read_cmd(&read_lux_data);
              current_state = i2c_calculate;
            }
          break;
        case i2c_calculate:
          if(evt->data.evt_system_external_signal.extsignals  == evti2ctransfercomplete)
            {
              NVIC_DisableIRQ(I2C0_IRQn);
              sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
              ambient_light_measurement(read_lux_data);
             // LOG_INFO("i2c cal Before func pir =%d\n\r",pir_count);

             // LOG_INFO("i2c cal after func pir =%d\n\r",pir_count);
             // LOG_INFO("LUX VALUE is=%d C\n\r",read_lux_data);
              current_state = i2c_init;
            }
          break;
        default:
          break;
      }
    }
}
//}
//}
#else

void discovery_state_machine(sl_bt_msg_t *evt)
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
                                                            sizeof(lux_service),
                                                            (const uint8_t*)lux_service);
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
                                                           sizeof(lux_char),
                                                           (const uint8_t*)lux_char);
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
                                                          gattdb_lux_measurement,
                                                          sl_bt_gatt_indication);
          if(sc != SL_STATUS_OK)
            {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
            }
          else
            {
              current_state = PRIMARY_PIR_CHAR_UUID;
            }
      }
      break;
    case PRIMARY_PIR_CHAR_UUID:
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {
          sc = sl_bt_gatt_discover_primary_services_by_uuid(bleDataPtr->connection_handle,
                                                            sizeof(pir_service),
                                                            (const uint8_t*)pir_service);
          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() 1 returned != 0 status=0x%04x\n\r", (unsigned int)sc);
          }
          else
            {
              current_state =  DISCOVER_PIR_CHAR_UUID;
            }
      }
      break;
    case DISCOVER_PIR_CHAR_UUID:
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
       {

          sc = sl_bt_gatt_discover_characteristics_by_uuid(bleDataPtr->connection_handle,
                                                           bleDataPtr->service_handle,
                                                           sizeof(pir_char),
                                                           (const uint8_t*)pir_char);
          if(sc != SL_STATUS_OK)
            {
              LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() 1 returned != 0 status=0x%04x\n\r", (unsigned int)sc);
            }
          else
            {
              current_state =SET_CHAR_PIR_NOTIFY;
            }
      }
      break;
    case SET_CHAR_PIR_NOTIFY:
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {

          sc = sl_bt_gatt_set_characteristic_notification(bleDataPtr->connection_handle,
                                                          gattdb_IR_Detection,
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
#endif
