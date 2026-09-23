#pragma once

#include <cstdint>
#include <array>

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
    constexpr std::array<float, 3> K = {-2.1431F, -0.2186F, -0.0013F};
}
