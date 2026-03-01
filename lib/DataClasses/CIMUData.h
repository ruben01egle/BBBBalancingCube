/**
 * @author	Michael Meindl, Benjamin Spiegler
 * @date	23.03.22
 * @brief	Method definition for CIMUData.
 */
#ifndef CIMUDATA_H
#define CIMUDATA_H
#include <cstdint>

class CIMUData
{
public:
	union
	{
		struct
		{
			int16_t mW_x;
			int16_t mW_y;
			int16_t mW_z;
			int16_t mA_x;
			int16_t mA_y;
			int16_t mA_z;
		};
		struct
		{
			int16_t mPadding1;
			int16_t mPadding2;
			int16_t mDotPhi;		//! Sensor  Z-Angular Velocity []
			int16_t mDDotX;		//! Sensor  X-Acceleration []
			int16_t mDDotY;		//! Sensor  Y-Acceleration []
			int16_t mPadding3;
		};
	};
};

static_assert(sizeof(CIMUData) == 12, "CIMUData size mismatch! Check alignment.");

#endif
