/**
 * @file	CMPU9250.h
 * @author	Michael Meindl, Benjamin Spiegler
 * @date	23.03.22
 * @brief	Class definition to initialize and read the MPU9250-Sensors.
 */
#ifndef CMPU9250_H
#define CMPU9250_H
#include "Global.h"
#include "CIMUData.h"


class CMPU9250
{
public:
	//The init method cant be moved to the ctor, because a single instance is used to control multiple MPUs
	//This is the case because of the FD for the SPI-Interface
	void init(UInt8 gyro_config);
	bool readSensorData(CIMUData& data);
public:
	CMPU9250(const char* SPI_PAtH);
	CMPU9250(const CMPU9250&) = delete;
	CMPU9250& operator=(const CMPU9250&) = delete;
	~CMPU9250();
public:

	bool writeRegister(UInt8 addr, const UInt8& data);		//Helper method to write a register of the MPU9250-device
	bool readRegister(UInt8 addr, UInt8& data);				//Helper method to read a register of the MPU9250-device
	bool transfer(UInt8* tx_buf, UInt8* rx_buf, int length);		//Helper method to transmit data
private:
	int mSPIFD;
	//Compile-Time constants holding the chips register addresses
	static constexpr UInt8 sSELF_TEST_X_GYRO 	= 0x00U;
	static constexpr UInt8 sSELF_TEST_Y_GYRO 	= 0x01U;
	static constexpr UInt8 sSELF_TEST_Z_GYRO 	= 0x02U;
	static constexpr UInt8 sSELF_TEST_X_ACCEL	= 0x0DU;
	static constexpr UInt8 sSELF_TEST_Y_ACCEL	= 0x0EU;
	static constexpr UInt8 sSELF_TEST_Z_ACCEL	= 0x0FU;
    static constexpr UInt8 sXG_OFFSET_H			= 0x13U;
	static constexpr UInt8 sXG_OFFSET_L			= 0x14U;
	static constexpr UInt8 sYG_OFFSET_H			= 0x15U;
	static constexpr UInt8 sYG_OFFSET_L			= 0x16U;
	static constexpr UInt8 sZG_OFFSET_H			= 0x17U;
	static constexpr UInt8 sZG_OFFSET_L			= 0x18U;
	static constexpr UInt8 sSMPLRT_DIV			= 0x19U;
	static constexpr UInt8 sCONFIG				= 0x1AU;
	static constexpr UInt8 sGYRO_CONFIG			= 0x1BU;
	static constexpr UInt8 sACCEL_CONFIG		= 0x1CU;
	static constexpr UInt8 sACCEL_CONFIG2		= 0x1DU;
	static constexpr UInt8 sLP_ACCEL_ODR		= 0x1EU;
	static constexpr UInt8 sWOM_THR				= 0x1FU;
	static constexpr UInt8 sFIFO_EN				= 0x23U;
	static constexpr UInt8 sI2C_MST_CTRL		= 0x24U;
	static constexpr UInt8 sI2C_SLV0_ADDR		= 0x25U;
	static constexpr UInt8 sI2C_SLV0_REG		= 0x26U;
	static constexpr UInt8 sI2C_SLV0_CTRL		= 0x27U;
	static constexpr UInt8 sI2C_SLV1_ADDR		= 0x28U;
	static constexpr UInt8 sI2C_SLV1_REG		= 0x29U;
	static constexpr UInt8 sI2C_SLV1_CTRL		= 0x2AU;
	static constexpr UInt8 sI2C_SLV2_ADDR		= 0x2BU;
	static constexpr UInt8 sI2C_SLV2_REG		= 0x2CU;
	static constexpr UInt8 sI2C_SLV2_CTRL		= 0x2DU;
	static constexpr UInt8 sI2C_SLV3_ADDR		= 0x2EU;
	static constexpr UInt8 sI2C_SLV3_REG		= 0x2FU;
	static constexpr UInt8 sI2C_SLV3_CTRL		= 0x30U;
	static constexpr UInt8 sI2C_SLV4_ADDR		= 0x31U;
	static constexpr UInt8 sI2C_SLV4_REG		= 0x32U;
	static constexpr UInt8 sI2C_SLV4_DO			= 0x33U;
	static constexpr UInt8 sI2C_SLV4_CTRL		= 0x34U;
	static constexpr UInt8 sI2C_SLV4_DI			= 0x35U;
	static constexpr UInt8 sI2C_MST_STATUS		= 0x36U;
	static constexpr UInt8 sINT_PIN_CFG			= 0x37U;
	static constexpr UInt8 sINT_ENABLE			= 0x38U;
	static constexpr UInt8 sINT_STATUS			= 0x3AU;
	static constexpr UInt8 sACCEL_XOUT_H		= 0x3BU;
	static constexpr UInt8 sACCEL_XOUT_L		= 0x3CU;
	static constexpr UInt8 sACCEL_YOUT_H		= 0x3DU;
	static constexpr UInt8 sACCEL_YOUT_L		= 0x3EU;
	static constexpr UInt8 sACCEL_ZOUT_H		= 0x3FU;
	static constexpr UInt8 sACCEL_ZOUT_L		= 0x40U;
	static constexpr UInt8 sTEMP_OUT_H			= 0x41U;
	static constexpr UInt8 sTEMP_OUT_L			= 0x42U;
	static constexpr UInt8 sGYRO_XOUT_H			= 0x43U;
	static constexpr UInt8 sGYRO_XOUT_L			= 0x44U;
	static constexpr UInt8 sGYRO_YOUT_H			= 0x45U;
	static constexpr UInt8 sGYRO_YOUT_L			= 0x46U;
	static constexpr UInt8 sGYRO_ZOUT_H			= 0x47U;
	static constexpr UInt8 sGYRO_ZOUT_L			= 0x48U;
	static constexpr UInt8 sEXT_SENS_DATA_00	= 0x49U;
	static constexpr UInt8 sEXT_SENS_DATA_01	= 0x4AU;
	static constexpr UInt8 sEXT_SENS_DATA_02    = 0x4BU;
	static constexpr UInt8 sEXT_SENS_DATA_03    = 0x4CU;
	static constexpr UInt8 sEXT_SENS_DATA_04    = 0x4DU;
	static constexpr UInt8 sEXT_SENS_DATA_05    = 0x4EU;
	static constexpr UInt8 sEXT_SENS_DATA_06    = 0x4FU;
	static constexpr UInt8 sEXT_SENS_DATA_07    = 0x50U;
	static constexpr UInt8 sEXT_SENS_DATA_08    = 0x51U;
	static constexpr UInt8 sEXT_SENS_DATA_09    = 0x52U;
	static constexpr UInt8 sEXT_SENS_DATA_10    = 0x53U;
	static constexpr UInt8 sEXT_SENS_DATA_11    = 0x54U;
	static constexpr UInt8 sEXT_SENS_DATA_12    = 0x55U;
	static constexpr UInt8 sEXT_SENS_DATA_13    = 0x56U;
	static constexpr UInt8 sEXT_SENS_DATA_14    = 0x57U;
	static constexpr UInt8 sEXT_SENS_DATA_15    = 0x58U;
	static constexpr UInt8 sEXT_SENS_DATA_16    = 0x59U;
	static constexpr UInt8 sEXT_SENS_DATA_17    = 0x5AU;
	static constexpr UInt8 sEXT_SENS_DATA_18    = 0x5BU;
	static constexpr UInt8 sEXT_SENS_DATA_19    = 0x5CU;
	static constexpr UInt8 sEXT_SENS_DATA_20    = 0x5DU;
	static constexpr UInt8 sEXT_SENS_DATA_21    = 0x5EU;
	static constexpr UInt8 sEXT_SENS_DATA_22    = 0x5FU;
	static constexpr UInt8 sEXT_SENS_DATA_23    = 0x60U;
	static constexpr UInt8 sI2C_SLV0_DO			= 0x63U;
	static constexpr UInt8 sI2C_SLV1_DO			= 0x64U;
	static constexpr UInt8 sI2C_SLV2_DO			= 0x65U;
	static constexpr UInt8 sI2C_SLV3_DO			= 0x66U;
	static constexpr UInt8 sI2C_MST_DELAY_CTRL	= 0x67U;
	static constexpr UInt8 sSIGNAL_PATH_RESET	= 0x68U;
	static constexpr UInt8 sMOT_DETECT_CTRL		= 0x69U;
	static constexpr UInt8 sUSER_CTRL			= 0x6AU;
	static constexpr UInt8 sPWER_MGMT_1			= 0x6BU;
	static constexpr UInt8 sPWER_MGMT_2			= 0x6CU;
	static constexpr UInt8 sFIFO_COUNTH			= 0x72U;
	static constexpr UInt8 sFIFO_COUNTL			= 0x73U;
	static constexpr UInt8 sFIFO_R_W			= 0x74U;
	static constexpr UInt8 sWHO_AM_I			= 0x75U;
	static constexpr UInt8 sXA_OFFSET_H			= 0x77U;
	static constexpr UInt8 sXA_OFFSET_L			= 0x78U;
	static constexpr UInt8 sYA_OFFSET_H			= 0x7AU;
	static constexpr UInt8 sYA_OFFSET_L 		= 0x7BU;
	static constexpr UInt8 sZA_OFFSET_H			= 0x7DU;
	static constexpr UInt8 sZA_OFFSET_L			= 0x7EU;
};

#endif
