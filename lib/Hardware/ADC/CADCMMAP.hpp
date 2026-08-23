#ifndef CADCMMAP_HPP
#define CADCMMAP_HPP

#include <cstdint>
#include <array>
#include <queue>
#include <mutex>

#include "CADCConfig.hpp"

template <typename InterfaceIdentifier, typename InterfaceType> class CInterfaceManager;

class CADCMMAP
{
	friend class CInterfaceManager<uint8_t, CADCMMAP>;
public:
    enum class Status : uint8_t {
		OKAY,
		MMAP_ERROR,
		CONFIG_ERROR,
		NO_VALUE,
		BIT_ERROR
	};
private:
	CADCMMAP(uint8_t pStepNumber);
	CADCMMAP(const CADCMMAP&);
	CADCMMAP& operator=(const CADCMMAP);
public:
	~CADCMMAP();
	Status init(CADCConfig pADCConfig);
	Status readADC(uint16_t& pValue);

private:
	void readFifo();
	uint32_t readRegister(const uint32_t pAddrOffset);
	void writeRegister(const uint32_t pAddrOffset, const uint32_t pValue);
	void setBits(const uint32_t pAddrOffset, const uint32_t pBitMask);
	void clearBits(const uint32_t pAddrOffset, const uint32_t pBitMask);
	Status configStep();

private:
	static constexpr uint8_t NUM_STEPS = 16;
	bool mModuleInitialized;
	uint8_t mStepIdx;
	CADCConfig mADCConfig;
	uint8_t* mMapPtr;

	static std::array<std::queue<uint16_t>, NUM_STEPS> mValueQueues;
	// guards mValueQueues, which is shared by every CADCMMAP instance (one per ADC step)
	static std::mutex mQueueMutex;

private:
	static constexpr uint32_t ADDR_START				= 0x44E0D000U;
	static constexpr uint32_t ADDR_END					= 0x44E0F000U;
	static constexpr uint32_t MAP_SIZE					= ADDR_END - ADDR_START;

	static constexpr uint32_t MASK_STEP_IDX				= 0b11110000000000000000U;
	static constexpr uint32_t MASK_DATA					= 0b111111111111U;

	// STEPCONFIG bit field: select single-ended sampling mode with internal reference voltage
	static constexpr uint32_t STEPCONFIG_SEL_INP_MODE	= 0x08U << 15;

	static constexpr uint32_t OFFS_REVISION 			= 0x00;
	static constexpr uint32_t OFFS_SYSCONFIG			= 0x10;
	static constexpr uint32_t OFFS_IRQSTATUS_RAW		= 0x24;
	static constexpr uint32_t OFFS_IRQSTATUS			= 0x28;
	static constexpr uint32_t OFFS_IRQENABLE_SET		= 0x2C;
	static constexpr uint32_t OFFS_IREQENABLE_CLR		= 0x30;
	static constexpr uint32_t OFFS_IRQWAKEUP			= 0x34;
	static constexpr uint32_t OFFS_DMAENABLE_SET		= 0x38;
	static constexpr uint32_t OFFS_DMAENABLE_CLR		= 0x3C;
	static constexpr uint32_t OFFS_CTRL					= 0x40;
	static constexpr uint32_t OFFS_ADCSTAT				= 0x44;
	static constexpr uint32_t OFFS_ADCRANGE				= 0x48;
	static constexpr uint32_t OFFS_ADC_CLKDIV			= 0x4C;
	static constexpr uint32_t OFFS_ADC_MISC				= 0x50;
	static constexpr uint32_t OFFS_STEPENABLE			= 0x54;
	static constexpr uint32_t OFFS_IDLECONFIG			= 0x58;
	static constexpr uint32_t OFFS_TS_CHARGE_STEPCONFIG = 0x5C;
	static constexpr uint32_t OFFS_TS_CHARGE_DELAY		= 0x60;
	static constexpr std::array<uint32_t, NUM_STEPS> OFFS_STEPCONFIG = {
		0x64, 0x6C, 0x74, 0x7C,
		0x84, 0x8C, 0x94, 0x9C,
		0xA4, 0xAC, 0xB4, 0xBC,
		0xC4, 0xCC, 0xD4, 0xDC
	};

	static constexpr std::array<uint32_t, NUM_STEPS> OFFS_STEPDELAY = {
		0x68, 0x70, 0x78, 0x80,
		0x88, 0x90, 0x98, 0xA0,
		0xA8, 0xB0, 0xB8, 0xC0,
		0xC8, 0xD0, 0xD8, 0xE0
	};
	static constexpr uint32_t OFFS_FIFO0COUNT			= 0xE4;
	static constexpr uint32_t OFFS_FIFO0THRESHOLD		= 0xE8;
	static constexpr uint32_t OFFS_DMA0REQ				= 0xEC;
	static constexpr uint32_t OFFS_FIFO1COUNT			= 0xF0;
	static constexpr uint32_t OFFS_FIFO1THRESHOLD		= 0xF4;
	static constexpr uint32_t OFFS_DMA1REQ				= 0xF8;
	static constexpr uint32_t OFFS_FIFO0DATA			= 0x100;
	static constexpr uint32_t OFFS_FIFO1DATA			= 0x200;
};

#endif