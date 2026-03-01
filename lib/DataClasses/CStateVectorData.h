/**
 * @author	Michael Meindl
 * @date	5.12.2016
 * @brief	Structure to hold the values of the state vector
 */
#ifndef CSTATEVECTORDATA_H
#define CSTATEVECTORDATA_H

class CStateVectorData
{
public:
	float mPhi_A;			//! Phi-Value from the acceleration-estimate [rad]
	float mPhi_G;			//! Phi-Value from the gyroscope-integration [rad]
	float mPhi_C;			//! Phi-Value from the complementaryfilter [rad]
	float mPhi_d;			//! Phi__d-Value from the gyroscopes [rad/sec]
	float mPsi_d;			//! Psi__d-Vallue from the ADC	[rad/sec]
};

#endif
