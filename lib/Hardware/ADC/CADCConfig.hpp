#ifndef CADCCONFIG_HPP
#define CADCCONFIG_HPP

#include <cstdint>

class CADCConfig
{
public:
	enum class FIFOSel : uint32_t {
		FIFO0 = 0x00 << 26,
		FIFO1 = 0x01 << 26
	};
	enum class Mode : uint32_t {
		ONESHOT = 0x00,
		CONTINUOUS = 0x01
	};
	enum class Channel : uint32_t {
		AIN0 = 0x00 << 19,
		AIN1 = 0x01 << 19,
		AIN2 = 0x02 << 19,
		AIN3 = 0x03 << 19,
		AIN4 = 0x04 << 19,
		AIN5 = 0x05 << 19,
		AIN6 = 0x06 << 19
	};
	enum class Averaging : uint32_t {
		NO_AVG 	   = 0x00 << 2,
		AVG_2_SAM  = 0x01 << 2,
		AVG_4_SAM  = 0x02 << 2,
		AVG_8_SAM  = 0x03 << 2,
		AVG_16_SAM = 0x04 << 2
	};
	
    Mode mode;
    Channel channel;
    FIFOSel fifo;
    Averaging averaging;
    uint8_t sampleDelay;
    uint16_t openDelay;

};

#endif