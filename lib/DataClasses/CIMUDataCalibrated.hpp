#pragma once

class CIMUDataCalibrated
{
public:
	float mDotPhi;		//! Calibrated Z-Angular Velocity [rad/sec]
	float mDDotX;		//! Calibrated X-Acceleration [m/s^2]
	float mDDotY;		//! Calibrated Y-Acceleration [m/s^2]
};

static_assert(sizeof(CIMUDataCalibrated) == 12, "CIMUDataCalibrated size mismatch! Check alignment.");
