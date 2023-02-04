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
	//GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	/*Setting the LED0 pin to push pull configuration*/
	//GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);



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






