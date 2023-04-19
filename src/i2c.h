/*
 * i2c.h
 *
 *  Created on: 09-Feb-2023
 *      Author: srira
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"
#include "em_i2c.h"


#define SI7021_DEVICE_ADDR 0x40
#define TEMP_READ_CMD 0xF3

#define VEML6030_DEVICE_ADDR 0x48
#define LUX_READ_CMD 0x04


void I2C_init();
void i2c_write_cmd();
I2C_TransferReturn_TypeDef i2c_read_cmd();
//int read_temp_from_si7021();
void i2c_deinitialize();
int read_lux_from_veml6030();
#endif /* SRC_I2C_H_ */
