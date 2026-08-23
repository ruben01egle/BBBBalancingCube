/*
 * CPWMMMAP.h
 *
 *  Created on: May 26, 2023
 *      Author: Knut
 */

#ifndef CPWMMMAP_HPP
#define CPWMMMAP_HPP

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <cstdlib>
#include <optional>

#include <cstdint>
#include <string>
#include "CPWMModuleConfig.hpp"

template <typename InterfaceIdentifier, typename InterfaceType> class CInterfaceManager;

class CPWMMMAP {
	friend class CInterfaceManager<uint8_t, CPWMMMAP>;

public:
// to determine the correct channel-, chip- and pin-indices, check the current status of your machine, its generated dynamically
// TODO: more robust physical address

	enum class Status : uint8_t {
		OKAY,
		MMMAP_ERROR,
		FAILED_TO_ENABLE_DEVICE,
		PIN_ALREADY_IN_USE,
		MODULE_CONFIG_CONFLICT,
		INVALID_MODULE,
		DRIVER_ERROR,
		INVALID_PIN,
		INVALID_FREQUENCY
	};

private:
	// constructor: use for initial configuration of PWM:
	// 		--> pPWMModule: between 0 and 2;
	//		--> pPWMPin: between 0 and 1
	CPWMMMAP(uint8_t pPWMModule);
	CPWMMMAP(const CPWMMMAP&);
	CPWMMMAP& operator=(const CPWMMMAP&);
public:
	~CPWMMMAP();
	Status init(uint8_t pPWMPin, CPWMModuleConfig pModuleConfig);
	Status setDutyCycle(uint8_t pPWMPin, double pDutyCyclePercent);

private:
	Status setFrequency(uint32_t pFrequencyHz);
	void setHighLowActive(uint8_t pPWMPin, bool pActiveHigh);
	// The kernel assigns sysfs pwmchip indices in probe/overlay order, which is not guaranteed
	// to match the physical EPWM module number -- resolve it by matching the physical address.
	std::optional<std::string> resolvePwmChipIndex(uint8_t pPWMModule) const;

private:
	uint8_t* mapPtr;
	uint8_t mPWMModule;
	std::optional<CPWMModuleConfig> mModuleConfig[2];
	double mCurrentPrescaler;
	uint16_t mCurrentPeriod;
	uint16_t mCurrentDutyCycle;

	const uint32_t ADDR_START_CLKCTRL;
	const uint32_t ADDR_START_CTRLMOD;
	const uint32_t ADDR_START_PWM[3];
	const uint32_t MAP_SIZE_CLKCTRL;
	const uint32_t MAP_SIZE_CTRLMOD;
	const uint32_t MAP_SIZE_PWM;
	const uint32_t OFFS_EPWMSS_CLKCTRL[3];
	const uint32_t OFFS_CTRLMOD_PWM;
	const uint32_t PWM_OFFS;
	const uint8_t  OFFS_CLKCONFIG;

	// Time-Base Submodule
	const uint8_t OFFS_TBCTL;			// control
	const uint8_t OFFS_TBSTS;
	const uint8_t OFFS_TBPHS;			// phase
	const uint8_t OFFS_TBCNT;			// counter
	const uint8_t OFFS_TBPRD;			// period

	// Counter-Compare Submodule
	const uint8_t OFFS_CMPCTL;		// control
	const uint8_t OFFS_CMPA;			// Compare A
	const uint8_t OFFS_CMPB;			// Compare B

	// Action-Qualifier Submodule
	const uint8_t OFFS_AQCTLA;		// Control A
	const uint8_t OFFS_AQCTLB;		// Control B

	// Trip-Zone Submodule
	const uint8_t OFFS_TZCTL;			// Control
};

#endif
