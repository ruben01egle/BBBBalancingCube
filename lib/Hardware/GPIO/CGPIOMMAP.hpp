#ifndef CGPIOMMAP_HPP
#define CGPIOMMAP_HPP

#include <cstdint>
#include <mutex>

// The beaglebone contains 4 (0-3) GPIO-Modules, each of these modules
// contains 32 (0-31) pins. To specify the right GPIO-Pin in the constructor multpiply
// the module number with 32 and add pin number, or check datasheet
// https://itbrainpower.net/a-gsm/images/BeagleboneBlackP9HeaderTable.pdf

template <typename InterfaceIdentifier, typename InterfaceType> class CInterfaceManager;

class CGPIOMMAP {
	friend class CInterfaceManager<uint8_t, CGPIOMMAP>;
public:
	enum class Status : uint8_t {
		OKAY,
		MMAP_ERROR,
		CONFIG_ERROR
	};
private:
	CGPIOMMAP(uint8_t pGPIONumber);
	CGPIOMMAP(const CGPIOMMAP&);
	CGPIOMMAP& operator=(const CGPIOMMAP);
public:
	~CGPIOMMAP();
	Status init(bool pOutput);
	Status setHigh();
	Status setLow();
	bool getCurrentState();

private:
	uint8_t mGPIOModuleIndex;
	uint8_t mGPIOPinIndex;
	uint8_t* mapPtr;
	bool mOutput;

	const uint32_t ADDR_START_CM_PER;
	const uint32_t ADDR_START_CM_WKUP;
	const uint32_t ADDR_START_GPIOMOD[4];
	const uint32_t MAP_SIZE_CM_PER;
	const uint32_t MAP_SIZE_CM_WKUP;
	const uint32_t MAP_SIZE_GPIOMOD;
	const uint32_t OFFS_CM_GPIO[4];

	const uint32_t OFFS_SYSCONFIG;
	const uint32_t OFFS_OE;
	const uint32_t OFFS_DATA_IN;
	const uint32_t OFFS_CLEAR_DOUT;
	const uint32_t OFFS_SET_DOUT;

	// GPIO_OE is a single 32-bit register shared by all 32 pins of a module; guards the
	// read-modify-write against concurrent init() calls on pins of the same bank.
	static std::mutex mOEMutex;
};



#endif /* HARDWARE_CGPIOMMAP_H_ */
