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

#define POWER_UP_TIME 80000 // 80 milli sec in microseconds
#define TEMP_READ_WAIT_TIME 10800 // 10.8 milli sec microseconds
uint16_t read_data; // temperature data
uint8_t cmd_data; // temperature request command data
I2C_TransferReturn_TypeDef transferStatus; // Status of data transfer
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
I2C_TransferReturn_TypeDef i2c_write_cmd()
{

// Send Measure Temperature command

cmd_data = TEMP_READ_CMD;
transferSequence.addr = SI7021_DEVICE_ADDR << 1; // shift device address left
transferSequence.flags = I2C_FLAG_WRITE;
transferSequence.buf[0].data = &cmd_data; // pointer to data to write
transferSequence.buf[0].len = sizeof(cmd_data);
transferStatus = I2CSPM_Transfer (I2C0, &transferSequence);
  if (transferStatus != i2cTransferDone)
  {
      LOG_ERROR("I2CSPM_Transfer: I2C bus write of cmd=0x30 failed\n\r");
  }
  return transferStatus;
}

/*Function Name: i2c_write_cmd()
Function use: Read command to Temperature Sensor /
return type: void*/
I2C_TransferReturn_TypeDef i2c_read_cmd()
{
// Send Measure Temperature command

//cmd_data = TEMPERATURE_READ_CMD;
transferSequence.addr = SI7021_DEVICE_ADDR << 1; // shift device address left
transferSequence.flags = I2C_FLAG_READ;
transferSequence.buf[0].data = (uint8_t*)&read_data; // pointer to data to read
transferSequence.buf[0].len = sizeof(read_data);
transferStatus = I2CSPM_Transfer (I2C0, &transferSequence);
  if (transferStatus != i2cTransferDone)
  {
      LOG_ERROR("I2CSPM_Transfer: I2C bus read of cmd=0x30 failed\n\r");
  }
  return transferStatus;
}


/*Function Name: read_temp_from_si7021()
Function use: Read temperature from Si7021 sensor /
return type: void*/
int read_temp_from_si7021()
{
  I2C_init();//I2C init
  gpioSi7021sensorOn();//Enable sensor

  timerdelay(POWER_UP_TIME);//Delay timer for 80 ms
  I2C_TransferReturn_TypeDef  transfer_status;
  transfer_status = i2c_write_cmd();
  timerdelay(TEMP_READ_WAIT_TIME);//Delay timer for 10.8 ms

  transfer_status = i2c_read_cmd();
  if(transfer_status==i2cTransferDone)
  {
      int temperature = 0;
      uint16_t temp_read = read_data;
      temp_read = ((temp_read>>10)&0x3F);//Obtaining temperature LSB
      temp_read = (temp_read|(read_data<<8));//Obtaining MSB of temperature
      temperature = (175.72 * temp_read)/ 65536- 46.85;
      LOG_INFO("Si7021 Temperature in Deg Celsius=%dC\n\r",temperature);

  }
  gpioSi7021sensorOff();//Power Off
  i2c_deinitialize();
  return 1;
}


