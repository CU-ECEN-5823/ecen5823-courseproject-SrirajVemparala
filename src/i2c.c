/*******************************************************************************
 * i2c.c
 * Date:        10-02-2022
 * Author:      Raghu Sai Phani Sriraj Vemparala, raghu.vemparala@colorado.edu
 * Description: This file has oscillators related information
 *
 *
 *
 ******************************************************************************/
#include "sl_i2cspm.h"
#include "i2c.h"
#include "timer.h"
#include "gpio.h"
#define INCLUDE_LOG_DEBUG 1
#include "log.h"



uint8_t cmd_data; // temperature request command data
I2C_TransferSeq_TypeDef transferSequence; // Data transfer Sequence

/*Function Name: i2c_deinitialize()
Function use: Reset i2c after data received /
return type: void*/
void i2c_deinitialize()
{
  I2C_Reset(I2C0);
  I2C_Enable(I2C0,false);
  void gpioI2C_SCL_Disable();
  void gpioI2C_SDA_Disable();
 //LOG_INFO("Deinitilaisation exit");
}

/*Function Name: I2C_init()
Function use: Initialize I2C /
return type: void*/
void I2C_init()
{
  //uint32_t i2c_bus_frequency;
// Initialize the I2C hardware
I2CSPM_Init_TypeDef I2C_Config = {
 .port = I2C0,
 .sclPort = gpioPortC,
 .sclPin = 10,
 .sdaPort = gpioPortC,
 .sdaPin = 11,
 .portLocationScl = 14,
 .portLocationSda = 16,
 .i2cRefFreq = 0,
 .i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
 .i2cClhr = i2cClockHLRStandard
 };
 I2CSPM_Init(&I2C_Config);
 //i2c_bus_frequency = I2C_BusFreqGet(I2C0);
}

/*Function Name: i2c_write_cmd()
Function use: Write command to Temperature Sensor /
return type: void*/
void i2c_write_cmd()
{

// Send Measure Temperature command
  I2C_TransferReturn_TypeDef     transferStatus;
cmd_data = TEMP_READ_CMD;
transferSequence.addr = SI7021_DEVICE_ADDR << 1; // shift device address left
transferSequence.flags = I2C_FLAG_WRITE;
transferSequence.buf[0].data = &cmd_data; // pointer to data to write
transferSequence.buf[0].len = sizeof(cmd_data);
NVIC_EnableIRQ(I2C0_IRQn);
//LOG_INFO("i2c_wri_cmd\n\r");
transferStatus = I2C_TransferInit(I2C0,&transferSequence);
  if(transferStatus < 0)
  {
      LOG_ERROR("I2C TransferInitialisation status %x write: Fail\n\r", (uint32_t)transferStatus);
  }
//transferStatus = I2C_Transfer(I2C0);
}

/*Function Name: i2c_write_cmd()
Function use: Read command to Temperature Sensor /
return type: void*/
I2C_TransferReturn_TypeDef i2c_read_cmd(uint16_t *read_data)
{
  I2C_TransferReturn_TypeDef transferStatus = 0;
// Send Measure Temperature command

//cmd_data = TEMPERATURE_READ_CMD;
transferSequence.addr = SI7021_DEVICE_ADDR << 1; // shift device address left
transferSequence.flags = I2C_FLAG_READ;
transferSequence.buf[0].data = (uint8_t*)read_data; // pointer to data to read
transferSequence.buf[0].len = 2;
//I2C_TransferInit(I2C0, &transferSequence);

NVIC_EnableIRQ(I2C0_IRQn);
//LOG_INFO("i2c read_cmd\n\r");
transferStatus = I2C_TransferInit(I2C0,&transferSequence);
if(transferStatus < 0) {
    LOG_ERROR("I2C TransferInitialisation status %x write: Fail\n\r", (uint32_t)transferStatus);
}

  return transferStatus;
}

