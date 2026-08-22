/*
 * CSPIChannelConfig.h
 *
 *  Created on: Aug 19, 2023
 *      Author: Knut
 */

#ifndef CSPICHANNELCONFIG_HPP
#define CSPICHANNELCONFIG_HPP

#include <cstdint>

// CSPIChannelConfig: use this class to specify the settings
// which shall be applied to a single spi channel.

class CSPIChannelConfig {
public:
	enum ESPIChipSelectTimingCtrl {
		CS_05,	// delay is 0.5*clock cycle
		CS_15,	// delay is 1.5*clock cycle
		CS_25,	// delay is 2.5*clock cycle
		CS_35,	// delay is 3.5*clock cycle
	};
	
	enum ESPIStartbitSelections {
		NO_STARTBIT,
		HIGH_STARTBIT,
		LOW_STARTBIT
	};

public:
	// CLOCK SETTINGS
	// SCLK_Frequency: frequencies are supported within the range
	// 11.719 Hz until 48.000.000 Hz
	int sclk_Frequency_Hz;

	// SCLKHighActive: polarity of clock signal (true = SPI-Modes 0/1) high active
	// or low active (false = SPI-Modes 2/3)
	bool sclkHighActive;

	// samplingOnEvenEdge: use this variable to specify, when the sampling
	// shall occur: on even edges (true = SPI-Modes 1/3) of SCLK or on odd edges
	// (false = SPI-Modes 0/2)
	bool samplingOnEvenEdge;

	// CHIP-SELECT-SETTINGS (cs)
	// polarity of chip select
	bool csHighActive;

	// maintain chip select active between words
	bool csMaintainActive;

	// timing of chip select: the time delay between chip select assertion
	// and the first clock edge and between the last clock edge and the
	// chip select removal
	ESPIChipSelectTimingCtrl csTiming;

	// ADDITIONAL SETTINGS
	// start bit polarity: this option allows to add an additional bit
	// prior to the word
	ESPIStartbitSelections startBitSelection;

	// word length: 4 to 32 bit is supported
	uint8_t wordLength;
};



#endif
