/*
 * CPWMMMAP.cpp
 *
 *  Created on: May 26, 2023
 *      Author: Knut
 */

#include "CPWMMMAP.hpp"

#include <string>
#include <cstring>
using namespace std;

#include "CErrorReporter.hpp"

CPWMMMAP::CPWMMMAP(uint8_t pPWMModule) :
							mapPtr(NULL),
							mPWMModule(pPWMModule),
							mModuleConfig{std::nullopt, std::nullopt},
							mCurrentPrescaler(0.0f),
							mCurrentPeriod(0U),
							mCurrentDutyCycle(0U),
							ADDR_START_CLKCTRL	(0x44E00000U),
							ADDR_START_CTRLMOD	(0x44E10000U),
							ADDR_START_PWM		{0x48300000U, 0x48302000U, 0x48304000U},
							MAP_SIZE_CLKCTRL	(0x400U),
							MAP_SIZE_CTRLMOD	(0x2000U),
							MAP_SIZE_PWM		(0x260U),
							OFFS_EPWMSS_CLKCTRL {0xD4U, 0xCCU, 0xD8U},
							OFFS_CTRLMOD_PWM	(0x664U),
							PWM_OFFS			{0x200U},
							OFFS_CLKCONFIG		(0x8U),
							OFFS_TBCTL			(0x0U),
							OFFS_TBSTS			(0x2U),
							OFFS_TBPHS			(0x6U),
							OFFS_TBCNT			(0x8U),
							OFFS_TBPRD			(0xAU),
							OFFS_CMPCTL			(0xEU),
							OFFS_CMPA			(0x12U),
							OFFS_CMPB			(0x14U),
							OFFS_AQCTLA			(0x16U),
							OFFS_AQCTLB			(0x18U),
							OFFS_TZCTL			(0x28U) 
{}

CPWMMMAP::~CPWMMMAP() {
	// delete mapping and check if operation is successful
	if(munmap(mapPtr, MAP_SIZE_PWM) == -1) {
		// print error message
		REPORT_ERROR_ERRNO("unable to delete pwm register mapping");
	}
}

CPWMMMAP::Status CPWMMMAP::init(uint8_t pPWMPin, CPWMModuleConfig pModuleConfig)
{
	// mmap to required memory space
	int mMemoryFD = open("/dev/mem", O_RDWR | O_SYNC);

	// mmap registers for required clock module
	mapPtr = reinterpret_cast<uint8_t*>(mmap(0,					// start address for new mapping
			MAP_SIZE_CLKCTRL,									// mapped length
			PROT_READ | PROT_WRITE,								// permit read and write operations
			MAP_SHARED,											// share changes with other processes
			mMemoryFD,											// mem filedescriptor
			ADDR_START_CLKCTRL));								// start address in mem file

	if(mapPtr == ((uint8_t*)-1)) {								// check for successful mapping
		// print error message
		REPORT_ERROR_ERRNO("unable to mmap clock module peripheral registers");
		return Status::MMMAP_ERROR;
	}

	// enable device
	*reinterpret_cast<uint32_t*>(mapPtr+OFFS_EPWMSS_CLKCTRL[mPWMModule]) |= 0x2;

	// wait until device is enabled, abort if this takes longer than 1 second.
	int counter = 0;
	while((*reinterpret_cast<uint32_t*>(mapPtr+OFFS_EPWMSS_CLKCTRL[mPWMModule]) & 0x00030000) != 0x0) {
		usleep(1);

		counter++;
		if(counter > 1E6) {
			REPORT_ERROR("can not enable EPWMSS clock module!");
			return Status::FAILED_TO_ENABLE_DEVICE;
		}
	}

	// delete mapping and check if operation is successful
	if(munmap(mapPtr, MAP_SIZE_CTRLMOD) == -1) {
		// print error message
		REPORT_ERROR_ERRNO("unable to delete clock module peripheral register mapping");
		return Status::MMMAP_ERROR;
	}

	// mmap registers for Control Module
	mapPtr = reinterpret_cast<uint8_t*>(mmap(0,					// start address for new mapping
			MAP_SIZE_CTRLMOD,									// mapped length
			PROT_READ | PROT_WRITE,								// permit read and write operations
			MAP_SHARED,											// share changes with other processes
			mMemoryFD,											// mem filedescriptor
			ADDR_START_CTRLMOD));								// start address in mem file

	if(mapPtr == ((uint8_t*)-1)) {								// check for successful mapping
		// print error message
		REPORT_ERROR_ERRNO("unable to mmap control modul registers");
		return Status::MMMAP_ERROR;
	}

	// enable clock for specified pwm chip
	*reinterpret_cast<uint32_t*>(mapPtr+OFFS_CTRLMOD_PWM) |= (0x1 << mPWMModule);
	//printf("%X\n", *reinterpret_cast<uint32_t*>(mapPtr+OFFS_CTRLMOD_PWM) & 0x7);

	// delete mapping and check if operation is successful
	if(munmap(mapPtr, MAP_SIZE_CTRLMOD) == -1) {
		// print error message
		REPORT_ERROR_ERRNO("unable to delete control modul register mapping");
		return Status::MMMAP_ERROR;
	}

	// mmap registers for PWM
	mapPtr = reinterpret_cast<uint8_t*>(mmap(0,					// start address for new mapping
			MAP_SIZE_PWM,										// mapped length
			PROT_READ | PROT_WRITE,								// permit read and write operations
			MAP_SHARED,											// share changes with other processes
			mMemoryFD,											// mem filedescriptor
			ADDR_START_PWM[mPWMModule]));						// start address in mem file

	if(mapPtr == ((uint8_t*)-1)) {								// check for successful mapping
		// print error message
		REPORT_ERROR_ERRNO("unable to mmap PWM");
		return Status::MMMAP_ERROR;
	}

	// close filedescriptor
	close(mMemoryFD);

	// enable pwm clock
	*reinterpret_cast<uint32_t*>(mapPtr + OFFS_CLKCONFIG) |= 0x111;


	// clear phase register
	*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBPHS) &= 0xFFFF0000;

	// clear counter register
	*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCNT) &= 0xFFFF0000;

	// initialize timer base settings:
	// [15-14]: 0x3: free-running, [12-7]: maintain current prescalers,
	// [6]: software-forced synchronization pulse off,
	// [5-4]: disable synchronization
	// [3]: no immediate register load
	// [2]: no phase register
	// [1-0]: up-count mode
	*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) &= uint16_t(0xDFB0);
	*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0xC030;

	// initalize counter-compare submodule: no immediate load,
	// reload shadow-registers if counter = 0
	*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_CMPCTL) &= uint16_t(0xFFA0);

	// disable Trip-Zone Submodule
	*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TZCTL) |= 0xF;

	if (mModuleConfig[pPWMPin].has_value()) {
		REPORT_ERROR("PWM Pin already configured!" + std::to_string(pPWMPin));
		return Status::PIN_ALREADY_IN_USE;
	}
	else if (mModuleConfig[(pPWMPin + 1) % 2].has_value() && mModuleConfig[(pPWMPin + 1) % 2].value() != pModuleConfig) {
		REPORT_ERROR("PWM Module configuration conflict" +std::to_string(mPWMModule));
		return Status::MODULE_CONFIG_CONFLICT;
	}
	
	// use pwm driver implementation from Michael Meindl for pre-configuration
	// and to enable the specified pwm pin
	string pwmNr = to_string(pPWMPin);
	string chipNr;
	switch(mPWMModule) {
	case 0:
		chipNr = "0";
		break;
	case 1:
		chipNr = "1";
		break;
	case 2:
		chipNr = "2";
		break;
	default:
		REPORT_ERROR("Invalid PWM Module");
		return Status::INVALID_MODULE;
	}
	string chip_path       = "/sys/class/pwm/pwmchip" + chipNr;
	string export_path 	   = chip_path + "/export";
	string pwm_path 	   = chip_path + "/pwm" + pwmNr;

	// check if directory exists
	int ret = access(pwm_path.c_str(), F_OK);
	if(ret < 0) {
		// if not: call export command
		int export_fd = open(export_path.c_str(), O_WRONLY);
		if(export_fd < 0) {
			REPORT_ERROR("Failed to open export.");
			return Status::DRIVER_ERROR;
		}

		ret = write(export_fd, pwmNr.c_str(), pwmNr.size());
		if(ret != static_cast<int>(pwmNr.size())) {
			REPORT_ERROR("Failed to export " + pwmNr);
			return Status::DRIVER_ERROR;
		}

		ret = close(export_fd);
		if(ret != 0) {
			REPORT_ERROR("Failed to close export.");
			return Status::DRIVER_ERROR;
		}
	}

	// configure the period
	string period_path = pwm_path + "/period";
	string periodStr = to_string(int64_t(1.0f/pModuleConfig.mFrequency * 1E9f));		// calculate required period
	int period_fd = open(period_path.c_str(), O_WRONLY);
	if(period_fd < 0) {
		REPORT_ERROR("Failed to open " + period_path);
		return Status::DRIVER_ERROR;
	}
	ret = write(period_fd, periodStr.c_str(), strlen(periodStr.c_str()));
	if(ret != static_cast<int>(strlen(periodStr.c_str()))) {
		REPORT_ERROR("Failed to write to period.");
		return Status::DRIVER_ERROR;
	}
	ret = close(period_fd);
	if(ret < 0) {
		REPORT_ERROR("Failed to close period " + period_path);
		return Status::DRIVER_ERROR;
	}

	// enable the pwm module by driver
	string enable_path = pwm_path + "/enable";
	int enable_fd = open(enable_path.c_str(), O_WRONLY);
	if(enable_fd < 0) {
		REPORT_ERROR("Failed to open " + enable_path);
		return Status::DRIVER_ERROR;
	}
	ret = write(enable_fd, "1", strlen("1"));
	ret = close(enable_fd);
	if(enable_fd < 0) {
		REPORT_ERROR("Failed to close " + enable_path);
		return Status::DRIVER_ERROR;
	}

	// set PWM-Frequency
	if (setFrequency(pModuleConfig.mFrequency) != Status::OKAY) {
		REPORT_ERROR("Error setting PWM frequency");
		return Status::INVALID_FREQUENCY;
	}

	// configure Action Qualifier Submodule
	setHighLowActive(pModuleConfig.mActiveHigh);

	mModuleConfig[pPWMPin] = pModuleConfig;
    return Status::OKAY;
}

CPWMMMAP::Status CPWMMMAP::setDutyCycle(uint8_t pPWMPin, double pDutyCyclePercent)
{
	// compute dutycycle register content
	mCurrentDutyCycle = mCurrentPeriod * pDutyCyclePercent / 100.0f;

	// set new duty cycle value
	switch(pPWMPin) {
	case 0:
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_CMPA) &= uint16_t(mCurrentDutyCycle);
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_CMPA) |= mCurrentDutyCycle;
		break;
	case 1:
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_CMPB) &= uint16_t(mCurrentDutyCycle);
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_CMPB) |= mCurrentDutyCycle;
		break;
	default:
		REPORT_ERROR("Invalid PWM-Pin");
		return Status::INVALID_PIN;
	}

	return Status::OKAY;
}

CPWMMMAP::Status CPWMMMAP::setFrequency(uint32_t pFrequencyHz) {
	// reset prescaler bit fields to 0x0
	*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) &= uint16_t(0xE07F);

	// use no prescaler at frequencies >= 65536*10ns = 655,36µs = 1525,88Hz
	if(pFrequencyHz >= 1526) {
		// no prescaler -> maintain Prescaler bit fields at 0
		mCurrentPrescaler = 1.0f;
	} else if(pFrequencyHz >= 763) {
		// prescaler 2
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0x80;
		mCurrentPrescaler = 2.0f;
	} else if(pFrequencyHz >= 382) {
		// prescaler 4
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0x100;
		mCurrentPrescaler = 4.0f;
	} else if (pFrequencyHz >= 255) {
		// prescaler 6
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0x180;
		mCurrentPrescaler = 6.0f;
	} else if (pFrequencyHz >= 191) {
		// prescaler 8
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0x200;
		mCurrentPrescaler = 8.0f;
	} else if (pFrequencyHz >= 153) {
		// prescaler 10
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0x280;
		mCurrentPrescaler = 10.0f;
	} else if (pFrequencyHz >= 128) {
		// prescaler 12
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0x300;
		mCurrentPrescaler = 12.0f;
	} else if (pFrequencyHz >= 109) {
		// prescaler 14
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0x380;
		mCurrentPrescaler = 14.0f;
	} else if (pFrequencyHz >= 96) {
		// prescaler 16
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0xC80;
		mCurrentPrescaler = 16.0f;
	} else if (pFrequencyHz >= 48) {
		// prescaler 32
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0x1080;
		mCurrentPrescaler = 32.0f;
	} else if (pFrequencyHz >= 24) {
		// prescaler 64
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0x1480;
		mCurrentPrescaler = 64.0f;
	} else if (pFrequencyHz >= 12) {
		// prescaler 128
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0x1880;
		mCurrentPrescaler = 128.0f;
	} else if (pFrequencyHz >= 6) {
		// prescaler 256
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBCTL) |= 0x1C80;
		mCurrentPrescaler = 256.0f;
	}
	else {
		REPORT_ERROR("Error setting PWM frequency: frequency must be > 6Hz");
		mCurrentPrescaler = -1.0f;
		return Status::INVALID_FREQUENCY;
	}

	// compute required period value
	mCurrentPeriod = 1.0f/(pFrequencyHz*mCurrentPrescaler*1E-8) - 1;

	// update period register content
	*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBPRD) &= uint16_t(0x0);
	*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_TBPRD) = mCurrentPeriod;

	return Status::OKAY;
}

// potentially different configuration for each pin possible, not supported for now
void CPWMMMAP::setHighLowActive(bool pActiveHigh) {
	// clear configuration registers: Disable all actions
	*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_AQCTLA) &= uint16_t(0xF000);
	*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_AQCTLB) &= uint16_t(0xF000);

	// configure action qualifier submodule
	if(pActiveHigh) {
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_AQCTLA) |= 0x12;
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_AQCTLB) |= 0x102;
	} 
	else {
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_AQCTLA) |= 0x24;
		*reinterpret_cast<uint16_t*>(mapPtr + PWM_OFFS + OFFS_AQCTLB) |= 0x24;
	} 
}
