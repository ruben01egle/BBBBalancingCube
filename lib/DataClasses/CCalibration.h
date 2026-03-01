#ifndef CCALIBRATION_H
#define CCALIBRATION_H

class CCalibration
{
public:
    // Accelerometer calibration factors
    float mImu1AccelScaleX;        // Multiplication factor for X-axis accelerometer
    float mImu1AccelScaleY;        // Multiplication factor for Y-axis accelerometer
    float mImu1AccelOffsetX;       // Offset for X-axis accelerometer
    float mImu1AccelOffsetY;       // Offset for Y-axis accelerometer

    // Gyroscope calibration factors
    float mImu1GyroScale;         // Multiplication factor for gyroscope
    float mImu1GyroOffset;        // Offset for gyroscope

    float mImu2AccelScaleX;
    float mImu2AccelScaleY;
    float mImu2AccelOffsetX;
    float mImu2AccelOffsetY;

    float mImu2GyroScale;
    float mImu2GyroOffset;

    // ADC calibration factors
    float mADCScale;
    float mADCOffset;

    // Angle offset
    float mPhiOffset;
};

#endif