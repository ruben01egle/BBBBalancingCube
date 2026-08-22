#include "CMPU9250.hpp"

#include <thread>
#include <chrono>

using namespace std;

#include "CInterfaceManager.hpp"
#include "CErrorReporter.hpp"

CMPU9250::CMPU9250(uint8_t pSPIModule, uint8_t pSPIChannel, CSPIChannelConfig pSPICfg):
                    mSPIModule(pSPIModule),
                    mSPIChannel(pSPIChannel),
                    mSPICfg(pSPICfg)
{

}

CMPU9250::Status CMPU9250::initImu(CMPU9250Setup pSetup)
{
    CInterfaceManager<uint8_t, CSPIModuleMMAP> interfaceManager;
	auto result = interfaceManager.getInstance(mSPIModule, true);
	if (result.has_value()) {
		mSPI = result.value();
	}
	else {
		return Status::FAILED_TO_CONFIGURE_IMU;
	}

    auto moduleStatus = mSPI->initModule();
	if (moduleStatus != CSPIModuleMMAP::Status::OKAY) {
		return Status::FAILED_TO_CONFIGURE_IMU;
	}

	auto channelStatus = mSPI->configChannel(mSPIChannel, mSPICfg);
	if (channelStatus != CSPIModuleMMAP::Status::OKAY) {
		return Status::FAILED_TO_CONFIGURE_IMU;
	}

    // H_RESET self-clears immediately, so its value can't be verified via readback.
    writeReg(PWR_MGMT_1, 0x80, false);
    this_thread::sleep_for(chrono::milliseconds(100));

    uint8_t whoAmI = 0;
	burstRead(WHO_AM_I, 1, &whoAmI);
    if (whoAmI != WHO_AM_I_EXPECTED) {
        REPORT_ERROR("WhoAmI not as expected: 0x", std::hex, static_cast<int>(whoAmI));
        return Status::FAILED_TO_CONFIGURE_IMU;
    }

    // wake up (clear SLEEP), CLKSEL=001 auto-selects PLL once ready, else internal oscillator
    if (writeReg(PWR_MGMT_1, 0x01) != Status::OKAY) {
        return Status::FAILED_TO_CONFIGURE_IMU;
    }
    // disable all sensors while configuring them
    if (writeReg(PWR_MGMT_2, 0x3F) != Status::OKAY) {
        return Status::FAILED_TO_CONFIGURE_IMU;
    }
    this_thread::sleep_for(chrono::milliseconds(50));

    // I2C_IF_DIS: force SPI-only primary interface
    if (writeReg(USER_CTRL, 0x10) != Status::OKAY) {
        return Status::FAILED_TO_CONFIGURE_IMU;
    }

    uint8_t gyroDlpfCfg = (pSetup.gyroLP == G_LP_OFF) ? 0x00 : pSetup.gyroLP;
    uint8_t gyroFchoiceB = (pSetup.gyroLP == G_LP_OFF) ? 0x03 : 0x00;
    if (writeReg(CONFIG, gyroDlpfCfg) != Status::OKAY) {
        return Status::FAILED_TO_CONFIGURE_IMU;
    }
    if (writeReg(GYRO_CONFIG, (pSetup.gyroScale << 3) | gyroFchoiceB) != Status::OKAY) {
        return Status::FAILED_TO_CONFIGURE_IMU;
    }

    if (writeReg(ACCEL_CONFIG, pSetup.accelScale << 3) != Status::OKAY) {
        return Status::FAILED_TO_CONFIGURE_IMU;
    }
    uint8_t accelFchoiceB = (pSetup.accelLP == A_LP_OFF) ? 0x08 : 0x00;
    uint8_t accelDlpfCfg = (pSetup.accelLP == A_LP_OFF) ? 0x00 : pSetup.accelLP;
    if (writeReg(ACCEL_CONFIG2, accelFchoiceB | accelDlpfCfg) != Status::OKAY) {
        return Status::FAILED_TO_CONFIGURE_IMU;
    }

    // re-enable all sensors
    if (writeReg(PWR_MGMT_2, 0x00) != Status::OKAY) {
        return Status::FAILED_TO_CONFIGURE_IMU;
    }
    this_thread::sleep_for(chrono::milliseconds(50));

    return Status::OKAY;
}

CMPU9250::Status CMPU9250::readImu(rawData &pData)
{
    constexpr uint8_t numBytes = 14;
    uint8_t rawData[numBytes] = {0};

    Status ret = burstRead(ACCEL_XOUT_H, numBytes, rawData);
    if (ret != Status::OKAY) {
        return Status::FAILED_TO_READ_SENSOR_DATA;
    }

    pData.xAccel = (rawData[0] << 8) | rawData[1];
    pData.yAccel = (rawData[2] << 8) | rawData[3];
    pData.zAccel = (rawData[4] << 8) | rawData[5];

    // rawData[6..7] hold TEMP_OUT_H/L, unused

    pData.xGyro  = (rawData[8] << 8)  | rawData[9];
    pData.yGyro  = (rawData[10] << 8) | rawData[11];
    pData.zGyro  = (rawData[12] << 8) | rawData[13];

    return Status::OKAY;
}

CMPU9250::Status CMPU9250::burstRead(uint8_t pStartAddr, uint8_t pNumBytes, uint8_t *pData)
{
    constexpr size_t MAX_BURST_SIZE = 64; 
    const uint8_t transLen = pNumBytes + 1;

    if (transLen > MAX_BURST_SIZE) {
        REPORT_ERROR("Burst-Read buffer overflow");
        return Status::FAILED_TO_READ_SENSOR_DATA;
    }

    uint32_t rx[MAX_BURST_SIZE] = {0};
    uint32_t tx[MAX_BURST_SIZE] = {0};

    tx[0] = READ | pStartAddr;

    auto status = mSPI->dataExchangeTxRx(mSPIChannel, tx, rx, transLen);
	if (status != CSPIModuleMMAP::Status::OKAY) {
		REPORT_ERROR("Imu burst read failed");
		return Status::FAILED_TO_READ_SENSOR_DATA;
	}

    for (uint8_t i = 1; i < transLen; ++i) {
        pData[i-1] = static_cast<uint8_t>(rx[i]);
    }

    return Status::OKAY;
}

CMPU9250::Status CMPU9250::writeReg(uint8_t pAddr, uint8_t pData, bool check)
{
    constexpr uint8_t transLen = 2;

    uint32_t rx[transLen] = {0};
    uint32_t tx[transLen] = {0};

    tx[0] = WRITE | pAddr;
    tx[1] = pData;

    auto status = mSPI->dataExchangeTxRx(mSPIChannel, tx, rx, transLen);
	if (status != CSPIModuleMMAP::Status::OKAY) {
		REPORT_ERROR("Imu write register failed");
		return Status::FAILED_TO_READ_SENSOR_DATA;
	}

    if (check) {
        uint8_t byte = 0;
        if (burstRead(pAddr, 1, &byte) != Status::OKAY || byte != pData) {
            REPORT_ERROR("Imu writeReg failed, expected 0x" , std::hex, static_cast<int>(pData), " received: ", std::hex, static_cast<int>(byte));
            return Status::FAILED_TO_READ_SENSOR_DATA;
        }
    }
    return Status::OKAY;
}
