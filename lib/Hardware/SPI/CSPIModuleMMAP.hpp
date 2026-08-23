/*
 * CSPIMMAP.h
 *
 *  Created on: Aug 19, 2023
 *      Author: Knut
 *
 *  The AM335x contains 2 SPI-Modules, each of them contains 2 channels.
 *  You should create only a single instance for a module, even if you
 *  want to use both channels. Simultaneous data transmission on both
 *  channels is not possible. Before calling CSPIModuleMMAP::configChannel cs is high
 *
 *  Pin information:
 *  SPI0:
 *  	-> SCLK: P9_22
 *  	-> MISO/AD0/D0: P9_21
 *  	-> MOSI/SDA/D1: P9_18
 *  	-> CS-Channel0: P9_17
 *  	-> CS-Channel1: [??? not available ???]
 *  SPI1:
 *  	-> SCLK: P9_31 / P9_42
 *  	-> MISO/AD0/D0: P9_29
 *  	-> MOSI/SDA/D1: P9_30
 *  	-> CS-Channel0: P9_28 / P9_20
 *  	-> CS-Channel1: P9_19 / P9_42
 *
 */
#pragma once

#include <cstdint>
#include "CSPIChannelConfig.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdlib>
#include <optional>
#include <ctime>


template <typename InterfaceIdentifier, typename InterfaceType> class CInterfaceManager;

class CSPIModuleMMAP {
	friend class CInterfaceManager<uint8_t, CSPIModuleMMAP>;

public:
	enum class Status : uint8_t {
		OKAY,
		MMAP_FAILED,
		FAILED_TO_ENABLE_DEVICE,
		FAILED_TO_CONFIGURE_CHANNEL,
		CHANNEL_NOT_AVAILABLE,
		TRANSFER_TIMEOUT,
	};

private:
	CSPIModuleMMAP(uint8_t pModuleIndex); // 0: SPI0; 1: SPI1
	CSPIModuleMMAP(const CSPIModuleMMAP&);
	CSPIModuleMMAP& operator=(const CSPIModuleMMAP&);
public:
	Status initModule();
	~CSPIModuleMMAP();
	void resetModule();
	Status configChannel(uint8_t pChannel, const CSPIChannelConfig& mChannelConfig);

	// dataExchangeTxRx: use 32-Bit array elements to pass tx and rx data even if you
	// have configured a shorter word length. To extract the returned data, you can use '&' to mask
	// the 32-bit word to your desired word length, for example: uint8_t mdata = 0xFF & rxData[0];
	Status dataExchangeTxRx(uint8_t pChannel, uint32_t* txData, uint32_t* rxData, int8_t numWordsToTransmit);
private:
	// busy-poll SPI_CHXSTAT[EOT] for pChannel until it equals expectedValue or the given
	// deadline (CLOCK_MONOTONIC) passes. returns false on timeout.
	bool waitForEOT(uint8_t pChannel, uint8_t expectedValue, const struct timespec& deadline);

	uint8_t mModuleIndex;
	bool mModuleInitialized;
	uint8_t* mapPtr;

	const uint32_t ADDR_START_SPI[2];
	const uint32_t ADDR_START_CM;
	const uint32_t MAP_SIZE_SPI;
	const uint32_t MAP_SIZE_CM;
	const uint32_t OFFS_CM_SPICLK[2];

	// spi module config
	const uint32_t SPI_SYSCONFIG;
	const uint32_t SPI_SYSSTATUS;
	const uint32_t SPI_IRQSTATUS;
	const uint32_t SPI_SYST;
	const uint32_t SPI_MODULCTRL;

	// spi channel config
	const uint32_t SPI_CHXCONF[2];
	const uint32_t SPI_CHXSTAT[2];
	const uint32_t SPI_CHXCTRL[2];
	const uint32_t SPI_TX[2];
	const uint32_t SPI_RX[2];

	std::optional<CSPIChannelConfig> mChannelConfig[2];
};
