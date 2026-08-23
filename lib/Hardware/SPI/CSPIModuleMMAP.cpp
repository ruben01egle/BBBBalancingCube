/*
 * CSPIModuleMMAP.cpp
 *
 *  Created on: Aug 21, 2023
 *      Author: Knut
 */

#include "CSPIModuleMMAP.hpp"

#include "CErrorReporter.hpp"

#include <ctime>

namespace {
	// how many status-register polls to do between wall-clock deadline checks;
	// amortizes clock_gettime's syscall cost across the busy-wait. Kept well above
	// the iteration count of the slowest in-scope healthy transfer (32-bit word @ 1MHz)
	// so a successful wait never triggers a clock_gettime call. Must stay a power of
	// two (checked via bitmask below) since this CPU has no hardware integer divide.
	constexpr uint32_t TIMEOUT_CHECK_INTERVAL_ITERATIONS = 1024;
	// a timeout now aborts the transfer, so bias generously against false positives
	// rather than detecting a genuine hang quickly
	constexpr uint32_t TIMEOUT_HEADROOM_MULTIPLIER = 20;
	constexpr uint32_t TIMEOUT_MIN_USEC_FLOOR = 1000;

	// CHXCONF bit 6: chip-select polarity idle default applied at module init, before any
	// per-channel config exists. Kept high for consistency with the kernel's spi driver default.
	constexpr uint32_t SPI_CS_IDLE_HIGH_BIT = 0x1U;

	// 48MHz reference clock divided into a 12-bit field (EXTCLK[7:0] | CLKD[3:0])
	constexpr uint32_t SPI_REF_CLK_HZ = 48000000U;
	constexpr uint32_t SPI_CLOCK_DIVIDER_MAX = 0xFFFU;
}

CSPIModuleMMAP::CSPIModuleMMAP(uint8_t pModuleIndex)
		: mModuleIndex(pModuleIndex),
		  mapPtr(NULL),
		  ADDR_START_SPI	{0x48030000U, 0x481A0000U},
		  ADDR_START_CM		(0x44E00000U),
		  MAP_SIZE_SPI		(0x1000U),
		  MAP_SIZE_CM		(0x400U),
		  OFFS_CM_SPICLK	{0x4CU, 0x50U},
		  SPI_SYSCONFIG		(0x110U),
		  SPI_SYSSTATUS		(0x114U),
		  SPI_IRQSTATUS		(0x118U),
		  SPI_SYST			(0x124U),
		  SPI_MODULCTRL		(0x128U),
		  SPI_CHXCONF		{0x12CU, 0x140U},
		  SPI_CHXSTAT		{0x130U, 0x144U},
		  SPI_CHXCTRL		{0x134U, 0x148U},
		  SPI_TX			{0x138U, 0x14CU},
		  SPI_RX			{0x13CU, 0x150U}
{
	mModuleInitialized = false;
}

CSPIModuleMMAP::Status CSPIModuleMMAP::initModule()
{
	if (mModuleIndex > 1) {
		REPORT_ERROR("Invalid SPI module index ", static_cast<int>(mModuleIndex));
		return Status::MMAP_FAILED;
	}

	if (!mModuleInitialized) {
		// configure mapping
		// mmap to required memory space for CM_PER
		int mMemoryFD = open("/dev/mem", O_RDWR | O_SYNC);
		if (mMemoryFD < 0) {
			REPORT_ERROR_ERRNO("unable to open /dev/mem");
			return Status::MMAP_FAILED;
		}

		// mmap registers for required clock module. Kept in a local variable rather than the
		// mapPtr member -- if any step below fails, the destructor must never end up calling
		// munmap(mapPtr, MAP_SIZE_SPI) against a mapping that was only ever MAP_SIZE_CM bytes.
		uint8_t* cmPtr = reinterpret_cast<uint8_t*>(mmap(0,					// start address for new mapping
				MAP_SIZE_CM,										// mapped length
				PROT_READ | PROT_WRITE,								// permit read and write operations
				MAP_SHARED,											// share changes with other processes
				mMemoryFD,											// mem filedescriptor
				ADDR_START_CM));									// start address in mem file

		if(cmPtr == MAP_FAILED) {								// check for successful mapping
			// print error message
			REPORT_ERROR_ERRNO("unable to mmap clock module peripheral registers");
			close(mMemoryFD);
			return Status::MMAP_FAILED;
		}

		// enable device
		*reinterpret_cast<volatile uint32_t*>(cmPtr+OFFS_CM_SPICLK[mModuleIndex]) |= 0x2;

		// wait until device is enabled, abort if this takes longer than 1 second.
		int counter = 0;
		while((*reinterpret_cast<volatile uint32_t*>(cmPtr+OFFS_CM_SPICLK[mModuleIndex]) & 0x00030000) != 0x0) {
			usleep(1);

			counter++;
			if(counter > 1E6) {
				REPORT_ERROR("can not enable spi module");
				munmap(cmPtr, MAP_SIZE_CM);
				close(mMemoryFD);
				return Status::FAILED_TO_ENABLE_DEVICE;
			}
		}

		// delete mapping and check if operation is successful
		if(munmap(cmPtr, MAP_SIZE_CM) == -1) {
			// print error message
			REPORT_ERROR_ERRNO("unable to delete clock module peripheral register mapping");
			close(mMemoryFD);
			return Status::MMAP_FAILED;
		}

		// mmap registers for SPI-Module
		mapPtr = reinterpret_cast<uint8_t*>(mmap(0,					// start address for new mapping
				MAP_SIZE_SPI,									// mapped length
				PROT_READ | PROT_WRITE,								// permit read and write operations
				MAP_SHARED,											// share changes with other processes
				mMemoryFD,											// mem filedescriptor
				ADDR_START_SPI[mModuleIndex]));				// start address in mem file

		close(mMemoryFD);

		if(mapPtr == MAP_FAILED) {								// check for successful mapping
			// print error message
			REPORT_ERROR_ERRNO("unable to mmap SPI-Module");
			return Status::MMAP_FAILED;
		}

		// reset module
		resetModule();

		// module configuration: prevent module from switching to idle mode,
		// both clocks active
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_SYSCONFIG) &= 0xFFFFFCE4;
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_SYSCONFIG) |= 0x308;

		// configure spi module as master with chip select usage
		// and single channel mode (this is required because CS will be
		// set and cleared manually
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_MODULCTRL) &= 0xFFFFFFF1;
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_MODULCTRL) |= 0x1;

		// for consisenty with linux spi driver: maintain standard cs high for idle
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[0]) |= (SPI_CS_IDLE_HIGH_BIT << 6);
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[1]) |= (SPI_CS_IDLE_HIGH_BIT << 6);

		mModuleInitialized = true;
	}

	return Status::OKAY;
}

CSPIModuleMMAP::~CSPIModuleMMAP()
{
	if (mapPtr != nullptr && mapPtr != MAP_FAILED) {
		// delete mapping and check if operation is successful
		if(munmap(mapPtr, MAP_SIZE_SPI) == -1) {
			// print error message
			REPORT_ERROR_ERRNO("unable to delete SPI-Module peripheral register mapping");
		}
	}
}

void CSPIModuleMMAP::resetModule() {
	// start reset operation
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_SYSCONFIG) |= 0x2;

	// wait until reset is finished
	while((*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_SYSSTATUS) & 0x1) != 1) {
		usleep(1000);
	}
	mModuleInitialized = false;
}

CSPIModuleMMAP::Status CSPIModuleMMAP::configChannel(uint8_t pChannel, const CSPIChannelConfig& pChannelConfig) {
	if (pChannel > 1) {
		REPORT_ERROR("Invalid Channel");
		return Status::CHANNEL_NOT_AVAILABLE;
	}
	if (mChannelConfig[pChannel].has_value()) {
		REPORT_ERROR("Channel already configured!");
		return Status::CHANNEL_NOT_AVAILABLE;
	}
	if (pChannelConfig.sclk_Frequency_Hz <= 0) {
		REPORT_ERROR("Invalid SCLK frequency (must be > 0)");
		return Status::FAILED_TO_CONFIGURE_CHANNEL;
	}
	if (pChannelConfig.wordLength < 4 || pChannelConfig.wordLength > 32) {
		REPORT_ERROR("Invalid word length (must be 4-32 bits)");
		return Status::FAILED_TO_CONFIGURE_CHANNEL;
	}

	// reset default value of register CH0CONF
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= 0x60000;
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) &= 0xC0000000;

	// set clock divider granularity to clock cycle granularity
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= 0x20000000;

	// set chip select timing control
	switch(pChannelConfig.csTiming) {
	case CSPIChannelConfig::ESPIChipSelectTimingCtrl::CS_05:
		// do not apply any changes, maintain default value
		break;
	case CSPIChannelConfig::ESPIChipSelectTimingCtrl::CS_15:
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= 0x2000000;
		break;
	case CSPIChannelConfig::ESPIChipSelectTimingCtrl::CS_25:
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= 0x4000000;
		break;
	case CSPIChannelConfig::ESPIChipSelectTimingCtrl::CS_35:
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= 0x6000000;
		break;
	default:
		REPORT_ERROR("Unexpected setting of chip select timing control!");
		return Status::FAILED_TO_CONFIGURE_CHANNEL;
	}

	// compute clock divider:
	uint32_t divider = SPI_REF_CLK_HZ / static_cast<uint32_t>(pChannelConfig.sclk_Frequency_Hz) - 1;
	if (divider > SPI_CLOCK_DIVIDER_MAX) {
		REPORT_ERROR("SCLK frequency out of supported range");
		return Status::FAILED_TO_CONFIGURE_CHANNEL;
	}
	int EXTCLK_content = 0x0 |
			((divider & 0xFF0) >> 4);
	int CLKD_content = 0x0 |
			(divider & 0x00F);
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= (CLKD_content << 2);
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCTRL[pChannel]) |= (EXTCLK_content << 8);

	// configure start bit and its polarity
	switch(pChannelConfig.startBitSelection) {
	case CSPIChannelConfig::ESPIStartbitSelections::NO_STARTBIT:
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) &= 0xFF7FFFFF;
		break;
	case CSPIChannelConfig::ESPIStartbitSelections::LOW_STARTBIT:
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= 0x800000;
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) &= 0xFEFFFFFF;
		break;
	case CSPIChannelConfig::ESPIStartbitSelections::HIGH_STARTBIT:
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= 0x1800000;
		break;
	default:
		REPORT_ERROR("start bit configuration is not valid");
		return Status::FAILED_TO_CONFIGURE_CHANNEL;
	}

	// configure data line 0 for input, line 1 for output
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= 0x10000;
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) &= 0xFFFBFFFF;
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) &= 0xFFFDFFFF;

	// configure word length selection
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= ((pChannelConfig.wordLength-1) << 7);

	// configure CS high/low active
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) &= ~(1 << 6); // Delete bit 6
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= (!pChannelConfig.csHighActive) << 6;
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_SYST) |= (pChannelConfig.csHighActive << pChannel);
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_SYST) &= ~(uint32_t(0x0) | (pChannelConfig.csHighActive << pChannel));

	// configure SPICLK high/low active
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= (!pChannelConfig.sclkHighActive) << 1;
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_SYST) |= (pChannelConfig.sclkHighActive << 6);
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_SYST) &= ~(uint32_t(0x0) | (pChannelConfig.sclkHighActive << 6));

	// configure sampling on even edges
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= pChannelConfig.samplingOnEvenEdge;

	mChannelConfig[pChannel] = pChannelConfig;
	return Status::OKAY;
}

bool CSPIModuleMMAP::waitForEOT(uint8_t pChannel, uint8_t expectedValue, const struct timespec& deadline) {
	uint32_t pollCount = 0;
	while(((*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXSTAT[pChannel])
					& 0x4) >> 2) != expectedValue) {
		if((++pollCount & (TIMEOUT_CHECK_INTERVAL_ITERATIONS - 1)) == 0) {
			struct timespec now;
			clock_gettime(CLOCK_MONOTONIC, &now);
			if(now.tv_sec > deadline.tv_sec ||
					(now.tv_sec == deadline.tv_sec && now.tv_nsec >= deadline.tv_nsec)) {
				return false;
			}
		}
	}
	return true;
}

CSPIModuleMMAP::Status CSPIModuleMMAP::dataExchangeTxRx(uint8_t pChannel, uint32_t *txData, uint32_t *rxData, int8_t numWordsToTransmit)
{
    // compute duration for a single word transfer
	uint32_t durationWordTransferUSec = 1E6f * mChannelConfig[pChannel].value().wordLength / ((float)mChannelConfig[pChannel].value().sclk_Frequency_Hz);
	uint32_t timeoutUSec = durationWordTransferUSec * TIMEOUT_HEADROOM_MULTIPLIER;
	if(timeoutUSec < TIMEOUT_MIN_USEC_FLOOR) {
		timeoutUSec = TIMEOUT_MIN_USEC_FLOOR;
	}

	// single syscall for the whole (healthy) transaction: every wait's deadline is derived
	// from this by pure arithmetic below, rather than re-querying the clock per wait.
	struct timespec deadline;
	clock_gettime(CLOCK_MONOTONIC, &deadline);
	auto advanceDeadline = [](struct timespec& ts, uint32_t usec) {
		ts.tv_nsec += static_cast<long>(usec) * 1000L;
		ts.tv_sec += ts.tv_nsec / 1000000000L;
		ts.tv_nsec %= 1000000000L;
	};

	// enable channel: CHXCTRL[EN]
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCTRL[pChannel]) |= 0x1;

	// activate manual CS for current channel CHXCONF[20]
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= 0x100000;

	Status result = Status::OKAY;
	int8_t i = 0;

	for(i = 0; i < numWordsToTransmit; i++) {
		// write content to TX register
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_TX[pChannel]) = txData[i];

		// wait for cleared End-of-Transfer-Bit (transfer started)
		advanceDeadline(deadline, timeoutUSec);
		if(!waitForEOT(pChannel, 0, deadline)) {
			REPORT_ERROR("SPI word transfer start timed out on channel ", (int)pChannel);
			result = Status::TRANSFER_TIMEOUT;
			break;
		}

		// wait for set End-of-Transfer-Bit (transfer complete)
		advanceDeadline(deadline, timeoutUSec);
		if(!waitForEOT(pChannel, 1, deadline)) {
			REPORT_ERROR("SPI word transfer completion timed out on channel ", (int)pChannel);
			result = Status::TRANSFER_TIMEOUT;
			break;
		}

		// handle cs: switch off between words if this is configured
		if(!mChannelConfig[pChannel].value().csMaintainActive) {
			*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) &= 0xFFEFFFFF;
		}

		// read RX register content and clear the register
		rxData[i] = (uint32_t(0x0) | (*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_RX[pChannel])));
		*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_RX[pChannel]) = 0x0;

		// enable cs only if it shall not be kept active and if a further word
		// shall be transmitted
		if(!mChannelConfig[pChannel].value().csMaintainActive && i < numWordsToTransmit - 1) {
			*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) |= 0x100000;
		}
	}

	// deactivate manual CS for current channel
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCONF[pChannel]) &= 0xFFEFFFFF;

	// disable channel
	*reinterpret_cast<volatile uint32_t*>(mapPtr+SPI_CHXCTRL[pChannel]) &= 0xFFFFFFFE;

	return result;
}
