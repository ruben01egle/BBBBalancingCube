/**
 * @file	CMPU9250.cpp
 * @author	Michael Meindl, Benjamin Spiegler
 * @date	23.03.22
 * @brief	Method definitions for CMPU9250.
 */
#include "CMPU9250.h"
#include "Assertion.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/spi/spidev.h>
#include <sys/time.h>
#include <errno.h>
#include <iostream>
using namespace std;

//#define SPI_PATH "/dev/spidev1.0"

CMPU9250::CMPU9250(const char* SPI_PATH) : mSPIFD(-1)
{
	//General setup of the SPI-Interface
	mSPIFD = open(SPI_PATH, O_RDWR);
	sAssertion(mSPIFD >= 0, "(CMPU9250::CMPU9250) Failed to open SPI-Device", true);

	UInt8 mode = 3;
	int ret = ioctl(mSPIFD, SPI_IOC_WR_MODE, &mode);
	sAssertion(ret != -1, "(CMPU9250::CMPU9250) Failed to configure the SPI Mode", true);

	UInt8 bits = 8;
	ret = ioctl(mSPIFD, SPI_IOC_WR_BITS_PER_WORD, &bits);
	sAssertion(ret != -1, "(CMPU9250::CMPU9250) Failed to configure the Write-Bits-Per-Word", true);

	ret = ioctl(mSPIFD, SPI_IOC_RD_BITS_PER_WORD, &bits);
	sAssertion(ret != -1, "(CMPU9250::CMPU9250) Failed to configure the Read-Bits-Per-Word", true);

	UInt32 lsb_flag = 0U;
	ret = ioctl(mSPIFD, SPI_IOC_WR_LSB_FIRST, &lsb_flag);
	sAssertion(ret != -1, "(CMPU9250::CMPU9250) Failed to set the LSB-Flag", true);

	ret = ioctl(mSPIFD, SPI_IOC_RD_LSB_FIRST, &lsb_flag);
	sAssertion(ret != -1, "(CMPU9250::CMPU9250) Failed to set the LSB-Flag", true);

	UInt32 speed = 1000000U;
	ret = ioctl(mSPIFD, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	sAssertion(ret != -1, "(CMPU9250::CMPU9250) Failed to set the max write speed", true);
}
void CMPU9250::init(UInt8 gyro_config)
{
	//Disable the I2C-Interface
	bool assertion = this->writeRegister(sUSER_CTRL, 0x10U);
	sAssertion(assertion, "(CMPU9250::CMPU9250) Failed to disable the MPU's I2C-Interface", true);

	//Set the Gyro's Digital-Lowpass Bandwidth of 20Hz
	assertion = this->writeRegister(sCONFIG, 0x04U);
	sAssertion(assertion, "(CMPU9250::CMPU9250) Failed to configure the Gyro-DLP", true);

	//Activate the Gyro's DLP and set its Full-Scale to gyro_config
	assertion = this->writeRegister(sGYRO_CONFIG, gyro_config);
	sAssertion(assertion, "(CMPU9250::CMPU9250) Failed to configure the Gyro", true);

	//Activate the Accelerometer's DLP and set its Full-Scale to +-2g checked ok. JW 24.4.22
	assertion = this->writeRegister(sACCEL_CONFIG, 0b00001000);
	sAssertion(assertion, "(CMPU9250::CMPU9250) Failed to configure the Accel", true);

	//Set the Accelerometer's DLP Bandwidth to 20Hz
	assertion = this->writeRegister(sACCEL_CONFIG2, 0x04U);
	sAssertion(assertion, "(CMPU9250::CMPU9250) Failed to configure the Accel-DLP", true);

	UInt8 whoami = 0U;
	assertion = this->readRegister(sWHO_AM_I, whoami);
	sAssertion(assertion, "(CMPU9250::CMPU9250) Failed to read the WHO_AM_I Register", false);

}
bool CMPU9250::writeRegister(UInt8 addr, const UInt8& data)
{
	UInt8 tx_buf[] = { addr, data };
	UInt8 rx_buf[] = { 0U, 0U };
	return this->transfer(tx_buf, rx_buf, 2);
}
bool CMPU9250::readRegister(UInt8 addr, UInt8& data)
{
	addr += UInt8(0x80U);
	UInt8 tx_buf[] = { addr, 0U };
	UInt8 rx_buf[] = { 0U, 0U };
	bool ret = this->transfer(tx_buf, rx_buf, 2);
	if(true == ret)
	{
		data = rx_buf[1];
	}
	return ret;
}
bool CMPU9250::transfer(UInt8* tx_buf, UInt8* rx_buf, int length)
{
	struct spi_ioc_transfer transfer;
	transfer.tx_buf 		= (unsigned long) tx_buf;
	transfer.rx_buf			= (unsigned long) rx_buf;
	transfer.len			= length;
	transfer.speed_hz		= 1000000;
	transfer.bits_per_word  = 8;
	transfer.delay_usecs    = 0;
	transfer.cs_change		= 0;
	transfer.pad			= 0;
	transfer.rx_nbits		= 0;
	transfer.tx_nbits		= 0;

	int ret = ioctl(mSPIFD, SPI_IOC_MESSAGE(1), &transfer);
	return ret >= 0;
}
bool CMPU9250::readSensorData(CIMUData& data)
{
	UInt8 tx_buf[] = { sACCEL_XOUT_H + 0x80U, sACCEL_XOUT_L + 0x80U,
					   sACCEL_YOUT_H + 0x80U, sACCEL_YOUT_L + 0x80U,
					   sACCEL_ZOUT_H + 0x80U, sACCEL_ZOUT_L + 0x80U,
					   sTEMP_OUT_H   + 0x80U, sTEMP_OUT_L   + 0x80U,
					   sGYRO_XOUT_H  + 0x80U, sGYRO_XOUT_L  + 0x80U,
					   sGYRO_YOUT_H  + 0x80U, sGYRO_YOUT_L  + 0x80U,
					   sGYRO_ZOUT_H  + 0x80U, sGYRO_ZOUT_L  + 0x80U,
					   0U };
	UInt8 rx_buf[15] = { 0U };
	bool ret = this->transfer(tx_buf, rx_buf, 15);
	if(true == ret)
	{
		data.mA_x = (static_cast<Int16>(rx_buf[1])  << 8) + static_cast<Int16>(rx_buf[2]);
		data.mA_y = (static_cast<Int16>(rx_buf[3])  << 8) + static_cast<Int16>(rx_buf[4]);
		data.mA_z = (static_cast<Int16>(rx_buf[5])  << 8) + static_cast<Int16>(rx_buf[6]);
		data.mW_x = (static_cast<Int16>(rx_buf[9])  << 8) + static_cast<Int16>(rx_buf[10]);
		data.mW_y = (static_cast<Int16>(rx_buf[11]) << 8) + static_cast<Int16>(rx_buf[12]);
		data.mW_z = (static_cast<Int16>(rx_buf[13]) << 8) + static_cast<Int16>(rx_buf[14]);
	}
	return ret;
}

CMPU9250::~CMPU9250() {
	close(mSPIFD);
}

