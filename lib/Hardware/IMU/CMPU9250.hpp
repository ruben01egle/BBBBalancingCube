#pragma once

#include <memory>

#include <cstdint>
#include "CSPIModuleMMAP.hpp"
#include "CSPIChannelConfig.hpp"

class CMPU9250 {
public:
    struct rawData {
        int16_t xAccel, yAccel, zAccel;
        int16_t xGyro, yGyro, zGyro;
    };

    enum class Status : uint8_t {
		OKAY,
		FAILED_TO_CONFIGURE_IMU,
		FAILED_TO_READ_SENSOR_DATA
	};

	enum EGyroConfig : uint8_t {
		FS_RANGE_250 = 0,
		FS_RANGE_500 = 1,
		FS_RANGE_1000 = 2,
		FS_RANGE_2000 = 3
	};

	enum EAccelConfig : uint8_t {
		FS_RANGE_2g = 0,
		FS_RANGE_4g = 1,
		FS_RANGE_8g = 2,
		FS_RANGE_16g = 3
	};

	// CONFIG register DLPF_CFG[2:0] (only active when GYRO_CONFIG.Fchoice_b == 0b00).
	// Also sets the temperature sensor's low-pass filter to the paired bandwidth.
	enum EGyroLP : uint8_t {
		G_LP_250HZ = 0,
		G_LP_184HZ = 1,
		G_LP_92HZ = 2,
		G_LP_41HZ = 3,
		G_LP_20HZ = 4,
		G_LP_10HZ = 5,
		G_LP_5HZ = 6,
		G_LP_3600HZ = 7,
		G_LP_OFF = 8	// bypasses the DLPF entirely (Fchoice_b = 0b11), ~8800Hz raw bandwidth
	};

	// ACCEL_CONFIG2 register A_DLPFCFG[2:0] (only active when ACCEL_FCHOICE_B == 0).
	enum EAccelLP : uint8_t {
		A_LP_460HZ = 0,
		A_LP_184HZ = 1,
		A_LP_92HZ = 2,
		A_LP_41HZ = 3,
		A_LP_20HZ = 4,
		A_LP_10HZ = 5,
		A_LP_5HZ = 6,
		A_LP_OFF = 8	// bypasses the DLPF entirely (ACCEL_FCHOICE_B = 1), ~1.13kHz raw bandwidth
	};

	struct CMPU9250Setup
    {
        EGyroConfig gyroScale;
        EAccelConfig accelScale;
        EGyroLP gyroLP;
        EAccelLP accelLP;
    };

public:
    CMPU9250(uint8_t pSPIModule, uint8_t pSPIChannel, CSPIChannelConfig pSPICfg);
    Status initImu(CMPU9250Setup pSetup);
    Status readImu(rawData &pData);

private:
	Status burstRead(uint8_t pStartAddr, uint8_t pNumBytes, uint8_t* pData);
	Status writeReg(uint8_t pAddr, uint8_t pData, bool check=true);

private:
    std::shared_ptr<CSPIModuleMMAP> mSPI;
	const uint8_t mSPIModule;
	const uint8_t mSPIChannel;
	const CSPIChannelConfig mSPICfg;

private:
    static constexpr uint8_t READ = 0x80;
	static constexpr uint8_t WRITE = 0x00;

	// MPU9250 register map (flat, no register banks)
	static constexpr uint8_t SELF_TEST_X_GYRO  =  0x00;
	static constexpr uint8_t SELF_TEST_Y_GYRO  =  0x01;
	static constexpr uint8_t SELF_TEST_Z_GYRO  =  0x02;
	static constexpr uint8_t SELF_TEST_X_ACCEL =  0x0D;
	static constexpr uint8_t SELF_TEST_Y_ACCEL =  0x0E;
	static constexpr uint8_t SELF_TEST_Z_ACCEL =  0x0F;
	static constexpr uint8_t XG_OFFSET_H       =  0x13;
	static constexpr uint8_t XG_OFFSET_L       =  0x14;
	static constexpr uint8_t YG_OFFSET_H       =  0x15;
	static constexpr uint8_t YG_OFFSET_L       =  0x16;
	static constexpr uint8_t ZG_OFFSET_H       =  0x17;
	static constexpr uint8_t ZG_OFFSET_L       =  0x18;
	static constexpr uint8_t SMPLRT_DIV        =  0x19;
	static constexpr uint8_t CONFIG            =  0x1A;
	static constexpr uint8_t GYRO_CONFIG       =  0x1B;
	static constexpr uint8_t ACCEL_CONFIG      =  0x1C;
	static constexpr uint8_t ACCEL_CONFIG2     =  0x1D;
	static constexpr uint8_t LP_ACCEL_ODR      =  0x1E;
	static constexpr uint8_t WOM_THR           =  0x1F;
	static constexpr uint8_t FIFO_EN           =  0x23;
	static constexpr uint8_t I2C_MST_CTRL      =  0x24;
	static constexpr uint8_t INT_PIN_CFG       =  0x37;
	static constexpr uint8_t INT_ENABLE        =  0x38;
	static constexpr uint8_t INT_STATUS        =  0x3A;
	static constexpr uint8_t ACCEL_XOUT_H      =  0x3B;
	static constexpr uint8_t ACCEL_XOUT_L      =  0x3C;
	static constexpr uint8_t ACCEL_YOUT_H      =  0x3D;
	static constexpr uint8_t ACCEL_YOUT_L      =  0x3E;
	static constexpr uint8_t ACCEL_ZOUT_H      =  0x3F;
	static constexpr uint8_t ACCEL_ZOUT_L      =  0x40;
	static constexpr uint8_t TEMP_OUT_H        =  0x41;
	static constexpr uint8_t TEMP_OUT_L        =  0x42;
	static constexpr uint8_t GYRO_XOUT_H       =  0x43;
	static constexpr uint8_t GYRO_XOUT_L       =  0x44;
	static constexpr uint8_t GYRO_YOUT_H       =  0x45;
	static constexpr uint8_t GYRO_YOUT_L       =  0x46;
	static constexpr uint8_t GYRO_ZOUT_H       =  0x47;
	static constexpr uint8_t GYRO_ZOUT_L       =  0x48;
	static constexpr uint8_t SIGNAL_PATH_RESET =  0x68;
	static constexpr uint8_t MOT_DETECT_CTRL   =  0x69;
	static constexpr uint8_t USER_CTRL         =  0x6A;
	static constexpr uint8_t PWR_MGMT_1        =  0x6B;
	static constexpr uint8_t PWR_MGMT_2        =  0x6C;
	static constexpr uint8_t FIFO_COUNTH       =  0x72;
	static constexpr uint8_t FIFO_COUNTL       =  0x73;
	static constexpr uint8_t FIFO_R_W          =  0x74;
	static constexpr uint8_t WHO_AM_I          =  0x75;
	static constexpr uint8_t XA_OFFSET_H       =  0x77;
	static constexpr uint8_t XA_OFFSET_L       =  0x78;
	static constexpr uint8_t YA_OFFSET_H       =  0x7A;
	static constexpr uint8_t YA_OFFSET_L       =  0x7B;
	static constexpr uint8_t ZA_OFFSET_H       =  0x7D;
	static constexpr uint8_t ZA_OFFSET_L       =  0x7E;

	static constexpr uint8_t WHO_AM_I_EXPECTED = 0x71;
};
