#pragma once

#include <string>
#include <vector>

#include "CCalibrationData.hpp"

class CCubeConfigLoader
{
public:
    // Enumerates this host's non-loopback IPv4 addresses and matches them against
    // the "ip" field of each entry in the JSON config file at pConfigPath. On the
    // first match, fills pOutCalibration and pOutCubeId from that entry and returns
    // true. On any failure (file missing/unreadable, malformed JSON, missing/wrong
    // -typed fields, or no IP match) reports a descriptive error and returns false,
    // leaving pOutCalibration/pOutCubeId unmodified.
    static bool loadForThisHost(const std::string& pConfigPath,
                                 CCalibrationData& pOutCalibration,
                                 int& pOutCubeId);

    // Finds this host's entry like loadForThisHost and overwrites only its 12 imu1/imu2
    // scale/offset fields with the values from pNewCalibration (ADC and phi offset are left
    // untouched). If pOutOldCalibration is given it receives the previous imu values. The file
    // is rewritten in place (keeps owner/permissions when running under sudo). Returns false
    // and reports an error on any failure, leaving the file unmodified.
    static bool updateImuCalibrationForThisHost(const std::string& pConfigPath,
                                                const CCalibrationData& pNewCalibration,
                                                CCalibrationData* pOutOldCalibration = nullptr);

    // Resolves the default config path: the directory containing the currently
    // running executable (via /proc/self/exe) joined with pFileName. Falls back to
    // pFileName relative to the current working directory if /proc/self/exe cannot
    // be resolved.
    static std::string defaultConfigPath(const std::string& pFileName = "cube_calibration.json");

private:
    static bool getLocalIPv4Addresses(std::vector<std::string>& pOutIPs);
};
