#ifndef CPWMMODULECONFIG_HPP
#define CPWMMODULECONFIG_HPP

#include <cstdint>

class CPWMModuleConfig
{
public:
    constexpr CPWMModuleConfig(uint32_t pFrequency, bool pActiveHigh):
        mFrequency(pFrequency),
        mActiveHigh(pActiveHigh)
        {};
    CPWMModuleConfig(const CPWMModuleConfig& other) = default;
    bool operator!=(const CPWMModuleConfig& other) const {
        return mFrequency != other.mFrequency || mActiveHigh != other.mActiveHigh;
    }

public:
    uint32_t mFrequency;
    bool mActiveHigh;
};

#endif