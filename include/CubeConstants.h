#include <cstdint>

#include "CCalibration.h"

namespace Cube
{
    constexpr int32_t TCP_PORT = 40000;
    constexpr float T_A = 0.02;
    constexpr float IMU_R1 = 0.14;
    constexpr float IMU_R2 = 0.061;
    constexpr float ALPHA = 0.98;

    constexpr CCalibration DEFAULT_CALIBRATION = {
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
        .mADCOffset        = 0.0
    };

    const uint8_t CUBE = 5;
    constexpr float K[] = {-2.1431F, -0.2186F, -0.0013F};
    static constexpr CCalibration ALL_CUBES[] = {
        // Index 0 = Cube 1
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = 0.1f, .mImu1AccelOffsetY = 0.08f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = -0.01f, .mImu2AccelOffsetY = 0.13f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.02f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = 0.0084f
        },
        // Index 1 = Cube 2
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = 0.20f, .mImu1AccelOffsetY = 0.12f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = 1.16f, .mImu2AccelOffsetY = 0.74f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.00409519f,
            .mADCScale = 0.076f, .mADCOffset = -156.0f, .mPhiOffset = -0.01f
        },
        // Index 2 = Cube 3
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = 0.02f, .mImu1AccelOffsetY = -0.02f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = 0.05f, .mImu2AccelOffsetY = 0.093f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.00409519f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = -0.01f
        },
        // Index 3 = Cube 4
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = 0.04f, .mImu1AccelOffsetY = 0.03f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = 0.16f, .mImu2AccelOffsetY = 0.17f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.00409519f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = -0.01f
        },
        // Index 4 = Cube 5
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = 0.1f, .mImu1AccelOffsetY = 0.11f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = 0.15f, .mImu2AccelOffsetY = -0.07f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.00409519f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = 0.01f
        },
        // Index 5 = Cube 6
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = 0.18f, .mImu1AccelOffsetY = 0.16f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = -0.13f, .mImu2AccelOffsetY = 0.16f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.00409519f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = -0.01f
        },
        // Index 6 = Cube 7
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = -0.02f, .mImu1AccelOffsetY = 0.0f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = -0.16f, .mImu2AccelOffsetY = 0.1f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.00409519f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = -0.01f
        },
        // Index 7 = Cube 8
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = 0.52f, .mImu1AccelOffsetY = 0.54f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = -0.10f, .mImu2AccelOffsetY = 0.08f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.00409519f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = -0.01f
        },
        // Index 8 = Cube 9
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = -0.1f, .mImu1AccelOffsetY = 0.0f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = -0.1f, .mImu2AccelOffsetY = 0.1f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.00409519f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = 0.05f
        },
        // Index 9 = Cube 10
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = -0.18f, .mImu1AccelOffsetY = 0.2f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = 0.34f, .mImu2AccelOffsetY = 0.27f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = 0.10f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = 0.01f
        },
        // Index 10 = Cube 11
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = 0.1f, .mImu1AccelOffsetY = 0.09f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = -0.004f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = 0.1f, .mImu2AccelOffsetY = 0.1f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = 0.0f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = 0.01f
        },
        // Index 11 = Cube 12
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = 0.1f, .mImu1AccelOffsetY = 0.08f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = -0.032442412f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = -0.01f, .mImu2AccelOffsetY = 0.13f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.00409519f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = 0.0f
        },
        // Index 12 = Cube 13
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = -0.015f, .mImu1AccelOffsetY = 0.16f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = 0.25f, .mImu2AccelOffsetY = 0.2f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.00409519f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = 0.04f
        },
        // Index 13 = Cube 14
        {
            .mImu1AccelScaleX = -0.0012f, .mImu1AccelScaleY = -0.0012f, .mImu1AccelOffsetX = -0.05f, .mImu1AccelOffsetY = 0.15f,
            .mImu1GyroScale = -0.001063686f, .mImu1GyroOffset = 0.01f,
            .mImu2AccelScaleX = -0.0012f, .mImu2AccelScaleY = -0.0012f, .mImu2AccelOffsetX = -0.24f, .mImu2AccelOffsetY = 0.12f,
            .mImu2GyroScale = -0.001063686f, .mImu2GyroOffset = -0.00409519f,
            .mADCScale = 0.076f, .mADCOffset = -155.0f, .mPhiOffset = 0.04f
        }
    };
}