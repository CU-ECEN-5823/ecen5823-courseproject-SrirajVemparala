/*******************************************************************************
 * scheduler.c
 * Date:        10-02-2022
 * Author:      Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
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
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"
// scheduler routine to set a scheduler event

#define POWER_UP_TIME 80000 // 80 milli sec in microseconds
#define TEMP_READ_WAIT_TIME 10800 // 10.8 milli sec microseconds

uint16_t read_data; // temperature data

I2C_TransferReturn_TypeDef transferStatus; // Status of data transfer

//Events present in the code
typedef enum
{
  evtReadTemperature = 1,
  evti2ccomp1setcomplete = 2,
  evti2ctransfercomplete = 4
}eventstriggered;

//States of I2C machines
enum I2C_states
{
  i2c_idle = 1,
  i2c_setup_time,
  i2c_write,
  i2c_read_wait,
  i2c_read
};

uint16_t myEvent=0;//Events that are triggered

/****************************************************************
 *@Function void schedulerSetEventTemperaturemeasurement()
 *@Description Called when COMP0 interrupt is triggered.
 *@Param NULL
 *@Return NULL
 ****************************************************************/
void schedulerSetEventTemperaturemeasurement()
{

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
 // LOG_INFO("Entering schedulerSetEventTemperaturemeasurement\n\r");
  sl_bt_external_signal(evtReadTemperature);
 // myEvent |=evtReadTemperature;
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
  sl_bt_external_signal(evti2ccomp1setcomplete);
  //myEvent |=evti2ccomp1setcomplete;
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
   sl_bt_external_signal(evti2ctransfercomplete);
   //myEvent |=evti2ctransfercomplete;
//   LOG_INFO("Entering schedulerSetEventi2cTransferDone\n\r");
   LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
   CORE_EXIT_CRITICAL();
}
/*******************************************************
 *@Function void getNextEvent()
 *@Description Obtain Next Event which is triggered
 *@Param NULL
 *@Return NULL
 *******************************************************/
//uint32_t getNextEvent()
//{
//  uint32_t theEvent = 0;
//
//      //IRQ Disabled
//      CORE_DECLARE_IRQ_STATE;
//      CORE_ENTER_CRITICAL();
//      if(myEvent & evtReadTemperature)
//      {
//      theEvent = evtReadTemperature; // 1 event to return to the caller
//      myEvent &=~evtReadTemperature;
//      }
//      else if(myEvent & evti2ccomp1setcomplete)
//      {
//        theEvent = evti2ccomp1setcomplete; // 1 event to return to the caller
//        myEvent &=~evti2ccomp1setcomplete;
//      }
//      else if(myEvent & evti2ctransfercomplete)
//      {
//        theEvent = evti2ctransfercomplete; // 1 event to return to the caller
//        myEvent &=~evti2ctransfercomplete;
//      }
//      CORE_EXIT_CRITICAL();
//
//  return theEvent;
//}

/*******************************************************
 *@Function void temperature_state_machine()
 *@Description State machine to communicate via i2c
 *@Param NULL
 *@Return NULL
 *******************************************************/
void temperature_state_machine(sl_bt_msg_t *evt)
{
  uint32_t eventValue = evt->data.evt_system_external_signal.extsignals;
  static uint32_t current_state = i2c_idle;

  switch(current_state)
  {
    case i2c_idle:
      if(eventValue== evtReadTemperature)
        {
         // LOG_INFO("Entering evtReadTemperature\n\r");
         // I2C_init();//I2C init
          gpioSi7021sensorOn();//Enable sensor
          current_state = i2c_setup_time;
          // update comp1 value
          //LOG_INFO("i2c_tr\n\r");
          timerwaitus_irq(POWER_UP_TIME);
        }
      break;
    case i2c_setup_time:
      if(eventValue == evti2ccomp1setcomplete)
        {
          current_state = i2c_write;
          I2C_init();//Initialize I2C
          i2c_write_cmd();//I2c write
          //sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);//Add sl power management
        }
      break;
    case i2c_write:
      //LOG_INFO("cw\n\r");
      if(eventValue == evti2ctransfercomplete)
        {
          NVIC_DisableIRQ(I2C0_IRQn);
          //LOG_INFO("i2c_w\n\r");
          current_state = i2c_read_wait;
          //sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
          //comp1 update value
          timerwaitus_irq(TEMP_READ_WAIT_TIME);
        }
      break;
    case i2c_read_wait://Wait for computation to complete
      if(eventValue == evti2ccomp1setcomplete)
        {
          //LOG_INFO("i2c_rw\n\r");
          current_state = i2c_read;
          i2c_read_cmd(&read_data);
          //sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
        }
      break;
    case i2c_read:
      if(eventValue == evti2ctransfercomplete)
      {
          NVIC_DisableIRQ(I2C0_IRQn);
          //sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
          //LOG_INFO("Entering i2c_read\n\r");
          uint32_t temperature = 0;
          uint16_t temp_read = read_data;
          temp_read = ((temp_read>>8));//Obtaining temperature LSB
          temp_read = (temp_read|(read_data<<8));//Obtaining MSB of temperature
         // LOG_INFO("temp_read is %x",temp_read);
          temperature = (175.72 * temp_read)/ 65536- 46.85;
          //LOG_INFO("Temp =%dC\n\r",temperature);
          ble_send_temp(temperature);
          gpioSi7021sensorOff();//Power Off
          i2c_deinitialize();
          current_state = i2c_idle;
       }
      break;
    default:
      break;
  }
}
