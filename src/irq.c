/*
 * irq.c
 * Date:        03-02-2022
 * Author:      Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
 *              Rajesh Srirangam,Rajesh.Srirangam@colorado.edu
 * Description: This file has interrupt handler related information
 *
 *
 *
 ******************************************************************************/
#include "irq.h"
#include "em_letimer.h"
#include "em_i2c.h"
#include  <stdbool.h>
#include "src/scheduler.h"
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"
#include "src/gpio.h"

//bool uf_int = true;//Check if underflow and COMP1 flag is set to control the LED ON and OFF
uint32_t log_timer = 0;
/*************
 * @Function void LETIMER0_IRQHandler()
 * @Description Used as LETIMER0 Interrupt Handler
 * @Param NULL
 * @Return NULL
 *************/
void LETIMER0_IRQHandler(void)
{
  uint32_t flag_value = 0;//Obtains interrupt flags
  flag_value = LETIMER_IntGetEnabled(LETIMER0);//Get flag info
  LETIMER_IntClear(LETIMER0,flag_value);
  //Check if COMP1 interrupt is set
  //Check if UF interrupt is set
  if((flag_value&LETIMER_IF_UF))
    {
      log_timer++;
      //Schedule the Event Temperature measurement
      schedulerSetEventAmbiencemeasurement();
    }
  if((flag_value&LETIMER_IF_COMP1))
    {
      LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);  //Disable COMP1 interrupt
      schedulerSetEventcomp1set();
    }
}
//PIR Sensor
//Author Rajesh
/*************
 * @Function void GPIO_EVEN_IRQHandler()
 * @Description Used as GPIO EVEN Interrupt Handler
 * @Param NULL
 * @Return NULL
 *************/
/**< GPIO_EVEN IRQ Handler */
void GPIO_EVEN_IRQHandler(void)
{
  uint32_t gpioInt = GPIO_IntGetEnabled();
  GPIO_IntClear(gpioInt);
  // GPIO_IntClear(0xFFFFFFFF);
  if (GPIO_PinInGet(PIR_SENSOR_PORT_1, PIR_SENSOR_PIN_1))
    {
      schedulerSetCountPIR_1_detect();
    }
  if (GPIO_PinInGet(PIR_SENSOR_PORT_2, PIR_SENSOR_PIN_2))
    {
      schedulerSetCountPIR_2_detect();
    }

  if (gpioInt==1<<PB0_pin)
    {
      if(GPIO_PinInGet(PB0_port,PB0_pin)==1)
        {
          //LOG_INFO("Button Press PB0\n\r");
          schedulerSetEventGPIOPB0set();
        }
      else
        {
          // LOG_INFO("Button Release PB0\n\r");
          schedulerSetEventGPIOPB0clear();
        }
    }
}
/*************
 * @Function void GPIO_EVEN_IRQHandler()
 * @Description Used as GPIO EVEN Interrupt Handler
 * @Param NULL
 * @Return NULL
 *************/
/**< GPIO_ODD IRQ Handler */
void GPIO_ODD_IRQHandler(void)
{
  uint32_t gpioIntodd = GPIO_IntGetEnabled();
  GPIO_IntClear(gpioIntodd);
  if (gpioIntodd==1<<PB1_pin)
    {
      if(GPIO_PinInGet(PB1_port,PB1_pin)==1)
        {
          //LOG_INFO("Button Press PB1\n\r");
          schedulerSetEventGPIOPB1set();
        }
      else  {
          // LOG_INFO("Button Release PB1\n\r");
          schedulerSetEventGPIOPB1clear();
      }
    }
}
/*************
 * @Function void I2C0_IRQHandler()
 * @Description Used as LETIMER0 Interrupt Handler
 * @Param NULL
 * @Return NULL
 *************/
void I2C0_IRQHandler(void) {
  // this can be locally defined
  I2C_TransferReturn_TypeDef  transferStatus;
  transferStatus = I2C_Transfer(I2C0);
  if (transferStatus == i2cTransferDone)
    {
      //LOG_INFO("i2cd\n\r");
      schedulerSetEventi2cTransferDone();
      //NVIC_DisableIRQ(I2C0_IRQn);//Should not be used in IRQ
    }
  if (transferStatus < 0)
    {
      LOG_ERROR("%d\n\r", transferStatus);
    }
  //LOG_INFO("TS=%d C\n\r",transferStatus);
}

int letimerMilliseconds()
{
  return (log_timer*3000);
}
