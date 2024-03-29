/*
   gpio.h

    Created on: Dec 12, 2018
        Author: Dan Walkes

    Updated by Dave Sluiter Sept 7, 2020. moved #defines from .c to .h file.
    Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

    Editor: Feb 26, 2022, Dave Sluiter
    Change: Added comment about use of .h files.

 *
 * @student     Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
 *              Rajesh Srirangam,Rajesh.Srirangam@colorado.edu

 */


// Students: Remember, a header file (a .h file) generally defines an interface
//           for functions defined within an implementation file (a .c file).
//           The .h file defines what a caller (a user) of a .c file requires.
//           At a minimum, the .h file should define the publicly callable
//           functions, i.e. define the function prototypes. #define and type
//           definitions can be added if the caller requires theses.


#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include "em_gpio.h"
#define PB0_port   gpioPortF
#define PB0_pin    6
#define PB1_port gpioPortF
#define PB1_pin  7
#define PIR_SENSOR_PORT_1  gpioPortD
#define PIR_SENSOR_PIN_1   10
#define PIR_SENSOR_PORT_2  gpioPortD
#define PIR_SENSOR_PIN_2   12
// Function prototypes
void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();
void gpioSi7021sensorOn();
void gpioSi7021sensorOff();
void gpioSi7021_SCL_Disable();
void gpioSi7021_SDA_Disable();
void gpioSetDisplayExtcomin(bool);


#endif /* SRC_GPIO_H_ */
