#pragma once

#include <cstdint>
#include <array>

#include "CCalibrationData.hpp"
#include "CPWMModuleConfig.hpp"
#include "CMaxonMotor.hpp"
#include "CSPIChannelConfig.hpp"
#include "CMPU9250.hpp"

namespace Cube
{
    constexpr int32_t TCP_PORT = 40000;
    constexpr float T_A = 0.02;
    constexpr float IMU_R1 = 0.061;
    constexpr float IMU_R2 = 0.14;
    constexpr float IMU_ALPHA = IMU_R1/IMU_R2;
    constexpr float ALPHA = 0.98;
    constexpr float MAX_TM = 0.1;

    constexpr CCalibrationData DEFAULT_CALIBRATION = {
        // IMU 1
        .mImu1AccelScaleX  = 1.0,
        .mImu1AccelScaleY  = 1.0,
        .mImu1AccelOffsetX = 0.0,
        .mImu1AccelOffsetY = 0.0,
        .mImu1GyroScale    = 1.0,
        .mImu1GyroOffset   = 0.0,

        // IMU 2
        .mImu2AccelScaleX  = 1.0,
        .mImu2AccelScaleY  = 1.0,
        .mImu2AccelOffsetX = 0.0,
        .mImu2AccelOffsetY = 0.0,
        .mImu2GyroScale    = 1.0,
        .mImu2GyroOffset   = 0.0,

        // ADC
        .mADCScale         = 1.0,
        .mADCOffset        = 0.0,

        .mPhiOffset        = 0.0
    };

    // Motor cfg
    constexpr uint8_t MOTOR_PWM_MODULE = 2;
    constexpr uint8_t MOTOR_PWM_PIN = 1;
    constexpr uint8_t MOTOR_ENABLE_GPIO = 66;
    constexpr uint8_t MOTOR_DIRECTION_GPIO = 67;
    constexpr CPWMModuleConfig MOTOR_PWM_MODULE_CFG = CPWMModuleConfig(50, true);

    constexpr double MOTOR_TORQUE_CONST = 0.0369;
    constexpr CMaxonMotor::PWMCfg MOTOR_PWM_CFG = {
        .dutyCyclePercentMin = 10,
        .dutyCyclePercentMax = 90,
        .minTarget = 0,
        .maxTarget = 2.0    // TODO: according to comment from old framework: maxCurrent: 3,21A bei 90% PWM. Max. Cont. eigentlich 2,7A
    };

    constexpr uint8_t ADC_STEP_IDX = 1;
    constexpr CADCConfig ADC_CFG = {
        .mode = CADCConfig::Mode::ONESHOT,
        .channel = CADCConfig::Channel::AIN0,
        .fifo = CADCConfig::FIFOSel::FIFO1,
        .averaging = CADCConfig::Averaging::AVG_4_SAM,
        .sampleDelay = 10,
        .openDelay = 20
    };

    // IMU cfg — both IMUs share SPI1, distinct CS channels
    constexpr uint8_t IMU_SPI_MODULE = 1;
    constexpr uint8_t IMU1_SPI_CHANNEL = 0;
    constexpr uint8_t IMU2_SPI_CHANNEL = 1;
    constexpr CSPIChannelConfig IMU_SPI_CHANNEL_CFG = {
        .sclk_Frequency_Hz = 1000000,
        .sclkHighActive = false,
        .samplingOnEvenEdge = true,
        .csHighActive = false,
        .csMaintainActive = true,
        .csTiming = CSPIChannelConfig::CS_15,
        .startBitSelection = CSPIChannelConfig::NO_STARTBIT,
        .wordLength = 8
    };
    constexpr CMPU9250::CMPU9250Setup IMU_SETUP = {
        .gyroScale = CMPU9250::FS_RANGE_2000,
        .accelScale = CMPU9250::FS_RANGE_4g,
        .gyroLP = CMPU9250::G_LP_20HZ,
        .accelLP = CMPU9250::A_LP_20HZ
    };

    // Calib cfg
    // TODO: ALL CUBES NEED CALIBRATING APART FROM 14
    constexpr uint8_t CUBE = 14;
    constexpr std::array<float, 3> K = {-2.1431F, -0.2186F, -0.0013F};
    static constexpr CCalibrationData ALL_CUBES[] = {
        // Index 0 = Cube 1
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 1 = Cube 2
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 2 = Cube 3
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 3 = Cube 4
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 4 = Cube 5
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 5 = Cube 6
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 6 = Cube 7
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 7 = Cube 8
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 8 = Cube 9
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 9 = Cube 10
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 10 = Cube 11
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 11 = Cube 12
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 12 = Cube 13
        {
            // IMU 1
            .mImu1AccelScaleX  = 1.0,
            .mImu1AccelScaleY  = 1.0,
            .mImu1AccelOffsetX = 0.0,
            .mImu1AccelOffsetY = 0.0,
            .mImu1GyroScale    = 1.0,
            .mImu1GyroOffset   = 0.0,

            // IMU 2
            .mImu2AccelScaleX  = 1.0,
            .mImu2AccelScaleY  = 1.0,
            .mImu2AccelOffsetX = 0.0,
            .mImu2AccelOffsetY = 0.0,
            .mImu2GyroScale    = 1.0,
            .mImu2GyroOffset   = 0.0,

            // ADC
            .mADCScale         = 1.0,
            .mADCOffset        = 0.0,

            .mPhiOffset        = 0.0
        },
        // Index 13 = Cube 14
        {
           // IMU 1
            .mImu1AccelScaleX  = 0.001198,
            .mImu1AccelScaleY  = 0.001198,
            .mImu1AccelOffsetX = -246.705326,
            .mImu1AccelOffsetY = 92.506873,
            .mImu1GyroScale    = 0.001064,
            .mImu1GyroOffset   = -38.782646,

            // IMU 2
            .mImu2AccelScaleX  = 0.001198,
            .mImu2AccelScaleY  = 0.001198,
            .mImu2AccelOffsetX = -96.891753,
            .mImu2AccelOffsetY = 80.694158,
            .mImu2GyroScale    = 0.001064,
            .mImu2GyroOffset   = -19.262887,

            // ADC
            .mADCScale         = 0.076,
            .mADCOffset        = 2038.0,

            .mPhiOffset        = -0.023
        }
    };
    constexpr auto CUBE_CONFIG = ALL_CUBES[CUBE - 1];
}