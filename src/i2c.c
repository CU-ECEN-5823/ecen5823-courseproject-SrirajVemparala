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
//#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#define INTEGRATION_TIME 100000 // 100 milli sec in microseconds

uint8_t cmd_data; // temperature request command data
I2C_TransferSeq_TypeDef transferSequence; // Data transfer Sequence
uint8_t read_cmd = 0x04;
uint8_t write_cmd = 0x00;
uint16_t data = 2048;
uint8_t* ALS_value;
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

void i2c_veml6030_write_cmd()
{

  // Send Measure Temperature command
  I2C_TransferReturn_TypeDef     transferStatus;
  //uint8_t send_data[2] = {(data&0xFF), (data>>8)&0xFF};
  transferSequence.addr = VEML6030_DEVICE_ADDR << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_WRITE_WRITE;
  transferSequence.buf[0].data = &write_cmd; // pointer to data to write
  transferSequence.buf[0].len = 1;
  transferSequence.buf[1].data = (uint8_t*)&data; // pointer to data to write
  transferSequence.buf[1].len = 2;
  NVIC_EnableIRQ(I2C0_IRQn);
  //transferStatus = I2CSPM_Transfer (I2C0, &transferSequence);
  transferStatus = I2C_TransferInit(I2C0, &transferSequence);
  if(transferStatus < 0)
    {
      LOG_ERROR("I2C TransferInitialisation status %x write: Fail\n\r", (uint32_t)transferStatus);
    }
 // return transferStatus;
}

/*Function Name: i2c_write_cmd()
Function use: Read command to Temperature Sensor /
return type: void*/
void i2c_veml6030_write_read_cmd(uint16_t *read_data)
{
  I2C_TransferReturn_TypeDef transferStatus = 0;
  // Send Measure Temperature command

  //cmd_data = TEMPERATURE_READ_CMD;
  transferSequence.addr = VEML6030_DEVICE_ADDR << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_WRITE_READ;
  transferSequence.buf[0].data = &read_cmd; // pointer to data to read
  transferSequence.buf[0].len = 1;
  transferSequence.buf[1].data = (uint8_t*)read_data; // pointer to data to read
  transferSequence.buf[1].len = 2;
  NVIC_EnableIRQ(I2C0_IRQn);
  //LOG_INFO("i2c read_cmd\n\r");
  //transferStatus = I2CSPM_Transfer (I2C0, &transferSequence);
  transferStatus = I2C_TransferInit(I2C0, &transferSequence);
  if(transferStatus < 0) {
      LOG_ERROR("I2C TransferInitialisation status %x write: Fail\n\r", (uint32_t)transferStatus);
  }
  //ALS_value=(uint8_t*)read_data;
  //return transferStatus;
}

/*Function Name: read_temp_from_si7021()
Function use: Read temperature from Si7021 sensor /
return type: void*/
/*int read_lux_from_veml6030()
{
  I2C_TransferReturn_TypeDef  transfer_status;
  uint16_t read_lux_data = 0;
  I2C_init();//I2C init
  transfer_status =  i2c_veml6030_write_cmd(0x00,2048);
  if(transfer_status==i2cTransferDone)
    {
      LOG_ERROR("PASS");
    }
    else
      {
        LOG_ERROR("I2C Transfer_status status %x write: Fail\n\r", (uint32_t)transfer_status);
      }
  timerdelay(INTEGRATION_TIME);//Delay timer for 100 ms
  transfer_status = i2c_veml6030_write_read_cmd(0x04,&read_lux_data);

  if(transfer_status==i2cTransferDone)
  {
      LOG_INFO("LUX VALUE is=%dC\n\r",read_lux_data);

  }
  else
    {
      LOG_ERROR("I2C Transfer_status status %x write: Fail\n\r", (uint32_t)transfer_status);
    }
  return 1;
}*/
