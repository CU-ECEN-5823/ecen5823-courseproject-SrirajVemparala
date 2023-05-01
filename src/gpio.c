/*
  gpio.c
 
    Created on: Dec 12, 2018
    Author: Dan Walkes
    Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.
   Dave Sluiter: Use this file to define functions that set up or control GPIOs.

 *
 * @student     Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
 *
 
 */


// *****************************************************************************
// Students:
// We will be creating additional functions that configure and manipulate GPIOs.
// For any new GPIO function you create, place that function in this file.
// *****************************************************************************

#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>

#include "gpio.h"


// Student Edit: Define these, 0's are placeholder values.
// See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
// and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
// to determine the correct values for these.

#define LED0_port  gpioPortF
#define LED0_pin   4
#define LED1_port  gpioPortF
#define LED1_pin   5
#define PB0_port  gpioPortF
#define PB0_pin   6
#define PB1_port  gpioPortF
#define PB1_pin   7
#define Si7021_EN_port gpioPortD
#define Si7021_EN_pin  15
#define Si7021_SCL_port gpioPortC
#define Si7021_SCL_pin  10
#define Si7021_SDA_port gpioPortC
#define Si7021_SDA_pin  11
#define LCD_PORT  gpioPortD
#define LCD_PIN   13


/*Function Name: gpioInit()
Function use: Set GPIO drive strengths and modes of operation /
return type: void*/
void gpioInit()
{

/*Setting the drive for the Port F to strong*/
	//GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
	/*Setting the drive for the Port F to weak*/
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	/*Setting the LED0 pin to push pull configuration*/
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

	//GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	/*Setting the LED0 pin to push pull configuration*/
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);
  GPIO_PinModeSet( Si7021_EN_port, Si7021_EN_pin, gpioModePushPull, true );
  GPIO_PinModeSet( PB0_port, PB0_pin, gpioModeInput, true );
 // GPIO_IntClear(0xFFFFFFFF);
  GPIO_ExtIntConfig(PB0_port, PB0_pin, PB0_pin, true, true, true);
  GPIO_PinModeSet( PB1_port, PB1_pin, gpioModeInput, true );
 // GPIO_IntClear(0xFFFFFFFF);
  GPIO_ExtIntConfig(PB1_port, PB1_pin, PB1_pin, true, true, true);
  GPIO_PinModeSet( PIR_SENSOR_PORT_1, PIR_SENSOR_PIN_1, gpioModeInputPullFilter, true );
  GPIO_ExtIntConfig(PIR_SENSOR_PORT_1, PIR_SENSOR_PIN_1, PIR_SENSOR_PIN_1, true, true, true);
  GPIO_PinModeSet( PIR_SENSOR_PORT_2, PIR_SENSOR_PIN_2, gpioModeInputPullFilter, true );
  GPIO_ExtIntConfig(PIR_SENSOR_PORT_2, PIR_SENSOR_PIN_2, PIR_SENSOR_PIN_2, true, true, true);
 // GPIO_PinModeSet(PIR_SENSOR_PORT, PIR_SENSOR_PIN, gpioModeInputPull, 1);
}

/*Function Name: gpioLed0SetOn()
Function use: Turn ON LED /
return type: void*/
void gpioLed0SetOn()
{
	GPIO_PinOutSet(LED0_port,LED0_pin);
}

/*Function Name: gpioLed0SetOff()
Function use: Turn OFF LED /
return type: void*/
void gpioLed0SetOff()
{
	GPIO_PinOutClear(LED0_port,LED0_pin);
}

/*Function Name: gpioLed1SetOn()
Function use: Turn ON LED /
return type: void*/
void gpioLed1SetOn()
{
	GPIO_PinOutSet(LED1_port,LED1_pin);
}

/*Function Name: gpioLed1SetOff()
Function use: Turn OFF LED /
return type: void*/
void gpioLed1SetOff()
{
	GPIO_PinOutClear(LED1_port,LED1_pin);
}

/*Function Name: gpioSi7021sensorOn()
Function use: Turn ON Sensor power supply /
return type: void*/
void gpioSi7021sensorOn()
{
  GPIO_PinOutSet(Si7021_EN_port,Si7021_EN_pin);
}

/*Function Name: gpioSi7021sensorOff()
Function use: Turn OFF Sensor power supply /
return type: void*/
void gpioSi7021sensorOff()
{
  GPIO_PinOutClear(Si7021_EN_port,Si7021_EN_pin);
}

/*Function Name: gpioSi7021_SCL_Disable()
Function use: Power Down SCL line /
return type: void*/
void gpioSi7021_SCL_Disable()
{
  GPIO_PinOutClear(Si7021_SCL_port,Si7021_SCL_pin);
}

/*Function Name: gpioSi7021_SDA_Disable()
Function use: Power Down SCL line /
return type: void*/
void gpioSi7021_SDA_Disable()
{
  GPIO_PinOutClear(Si7021_SDA_port,Si7021_SDA_pin);
}

void gpioSetDisplayExtcomin(bool extcomin_state)
{
  if(extcomin_state == true)
  {
      GPIO_PinOutSet(LCD_PORT, LCD_PIN);
  }
  else
  {
      GPIO_PinOutClear(LCD_PORT, LCD_PIN);
  }
}
