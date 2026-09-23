#include "CCubeConfigLoader.hpp"

#include <array>
#include <climits>
#include <cmath>
#include <cstring>
#include <fstream>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>

#include "nlohmann/json.hpp"
#include "CErrorReporter.hpp"

using json = nlohmann::json;

bool CCubeConfigLoader::getLocalIPv4Addresses(std::vector<std::string>& pOutIPs)
{
    ifaddrs* ifaddrList = nullptr;
    if (getifaddrs(&ifaddrList) != 0) {
        REPORT_ERROR_ERRNO("CCubeConfigLoader: getifaddrs failed");
        return false;
    }

    for (ifaddrs* ifa = ifaddrList; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family != AF_INET) {
            continue;
        }
        if (ifa->ifa_flags & IFF_LOOPBACK) {
            continue;
        }

        char buf[INET_ADDRSTRLEN];
        const auto* addrIn = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
        if (inet_ntop(AF_INET, &addrIn->sin_addr, buf, sizeof(buf)) != nullptr) {
            pOutIPs.emplace_back(buf);
        }
    }

    freeifaddrs(ifaddrList);
    return true;
}

std::string CCubeConfigLoader::defaultConfigPath(const std::string& pFileName)
{
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len < 0) {
        REPORT_ERROR_ERRNO("CCubeConfigLoader: readlink(/proc/self/exe) failed, falling back to CWD-relative path");
        return pFileName;
    }
    buf[len] = '\0';

    std::string exePath(buf);
    size_t lastSlash = exePath.find_last_of('/');
    if (lastSlash == std::string::npos) {
        return pFileName;
    }
    return exePath.substr(0, lastSlash + 1) + pFileName;
}

namespace {

const char* const IMU_KEYS[] = {
    "imu1AccelScaleX", "imu1AccelScaleY", "imu1AccelOffsetX", "imu1AccelOffsetY",
    "imu1GyroScale", "imu1GyroOffset",
    "imu2AccelScaleX", "imu2AccelScaleY", "imu2AccelOffsetX", "imu2AccelOffsetY",
    "imu2GyroScale", "imu2GyroOffset"
};

// Pointers to the 12 imu fields of a CCalibrationData, in IMU_KEYS order
template<typename CalibT>
std::array<decltype(&CalibT::mImu1AccelScaleX), 12> imuFields()
{
    return {&CalibT::mImu1AccelScaleX, &CalibT::mImu1AccelScaleY,
            &CalibT::mImu1AccelOffsetX, &CalibT::mImu1AccelOffsetY,
            &CalibT::mImu1GyroScale, &CalibT::mImu1GyroOffset,
            &CalibT::mImu2AccelScaleX, &CalibT::mImu2AccelScaleY,
            &CalibT::mImu2AccelOffsetX, &CalibT::mImu2AccelOffsetY,
            &CalibT::mImu2GyroScale, &CalibT::mImu2GyroOffset};
}

// Returns the entry of root["cubes"] whose "ip" matches one of pLocalIPs, or nullptr
// (after reporting an error) if the structure is malformed or nothing matches.
template<typename JsonT>
JsonT* findHostEntry(JsonT& pRoot, const std::string& pConfigPath,
                     const std::vector<std::string>& pLocalIPs, std::string& pOutIP)
{
    if (!pRoot.contains("cubes") || !pRoot["cubes"].is_array()) {
        REPORT_ERROR("CCubeConfigLoader: cube config file '", pConfigPath, "' is missing a 'cubes' array");
        return nullptr;
    }

    for (auto& entry : pRoot["cubes"]) {
        std::string entryIP;
        try {
            entryIP = entry.at("ip").template get<std::string>();
        } catch (const std::exception& e) {
            REPORT_ERROR("CCubeConfigLoader: malformed entry in '", pConfigPath, "' (missing/invalid 'ip'): ", e.what());
            return nullptr;
        }

        for (const auto& localIP : pLocalIPs) {
            if (localIP == entryIP) {
                pOutIP = entryIP;
                return &entry;
            }
        }
    }

    std::string localIPList;
    for (size_t i = 0; i < pLocalIPs.size(); ++i) {
        if (i > 0) localIPList += ", ";
        localIPList += pLocalIPs[i];
    }
    REPORT_ERROR("CCubeConfigLoader: none of this host's local IPs [", localIPList, "] match any entry in '", pConfigPath, "'");
    return nullptr;
}

} // namespace

bool CCubeConfigLoader::loadForThisHost(const std::string& pConfigPath,
                                         CCalibrationData& pOutCalibration,
                                         int& pOutCubeId)
{
    std::ifstream file(pConfigPath);
    if (!file.is_open()) {
        REPORT_ERROR("CCubeConfigLoader: could not open cube config file at '", pConfigPath, "'");
        return false;
    }

    json root;
    try {
        file >> root;
    } catch (const json::exception& e) {
        REPORT_ERROR("CCubeConfigLoader: failed to parse cube config file '", pConfigPath, "': ", e.what());
        return false;
    }

    std::vector<std::string> localIPs;
    if (!getLocalIPv4Addresses(localIPs) || localIPs.empty()) {
        REPORT_ERROR("CCubeConfigLoader: no non-loopback IPv4 addresses found on this host");
        return false;
    }

    std::string entryIP;
    const json* entry = findHostEntry(root, pConfigPath, localIPs, entryIP);
    if (entry == nullptr) {
        return false;
    }

    try {
        pOutCubeId = entry->at("cube").get<int>();
        const auto& calib = entry->at("calibration");
        pOutCalibration.mImu1AccelScaleX  = calib.at("imu1AccelScaleX").get<float>();
        pOutCalibration.mImu1AccelScaleY  = calib.at("imu1AccelScaleY").get<float>();
        pOutCalibration.mImu1AccelOffsetX = calib.at("imu1AccelOffsetX").get<float>();
        pOutCalibration.mImu1AccelOffsetY = calib.at("imu1AccelOffsetY").get<float>();
        pOutCalibration.mImu1GyroScale    = calib.at("imu1GyroScale").get<float>();
        pOutCalibration.mImu1GyroOffset   = calib.at("imu1GyroOffset").get<float>();
        pOutCalibration.mImu2AccelScaleX  = calib.at("imu2AccelScaleX").get<float>();
        pOutCalibration.mImu2AccelScaleY  = calib.at("imu2AccelScaleY").get<float>();
        pOutCalibration.mImu2AccelOffsetX = calib.at("imu2AccelOffsetX").get<float>();
        pOutCalibration.mImu2AccelOffsetY = calib.at("imu2AccelOffsetY").get<float>();
        pOutCalibration.mImu2GyroScale    = calib.at("imu2GyroScale").get<float>();
        pOutCalibration.mImu2GyroOffset   = calib.at("imu2GyroOffset").get<float>();
        pOutCalibration.mADCScale         = calib.at("adcScale").get<float>();
        pOutCalibration.mADCOffset        = calib.at("adcOffset").get<float>();
        pOutCalibration.mPhiOffset        = calib.at("phiOffset").get<float>();
    } catch (const json::exception& e) {
        REPORT_ERROR("CCubeConfigLoader: malformed calibration entry for ip '", entryIP, "' in '", pConfigPath, "': ", e.what());
        return false;
    }

    std::cout << "CCubeConfigLoader: matched local IP " << entryIP << " to cube " << pOutCubeId << std::endl;
    return true;
}

bool CCubeConfigLoader::updateImuCalibrationForThisHost(const std::string& pConfigPath,
                                                        const CCalibrationData& pNewCalibration,
                                                        CCalibrationData* pOutOldCalibration)
{
    using ordered_json = nlohmann::ordered_json;

    ordered_json root;
    {
        std::ifstream file(pConfigPath);
        if (!file.is_open()) {
            REPORT_ERROR("CCubeConfigLoader: could not open cube config file at '", pConfigPath, "'");
            return false;
        }
        try {
            file >> root;
        } catch (const ordered_json::exception& e) {
            REPORT_ERROR("CCubeConfigLoader: failed to parse cube config file '", pConfigPath, "': ", e.what());
            return false;
        }
    }

    std::vector<std::string> localIPs;
    if (!getLocalIPv4Addresses(localIPs) || localIPs.empty()) {
        REPORT_ERROR("CCubeConfigLoader: no non-loopback IPv4 addresses found on this host");
        return false;
    }

    std::string entryIP;
    ordered_json* entry = findHostEntry(root, pConfigPath, localIPs, entryIP);
    if (entry == nullptr) {
        return false;
    }

    const auto fields = imuFields<CCalibrationData>();
    CCalibrationData oldCalibration{};
    try {
        auto& calib = entry->at("calibration");
        for (size_t i = 0; i < fields.size(); ++i) {
            oldCalibration.*fields[i] = calib.at(IMU_KEYS[i]).get<float>();
            // Rounded to 6 decimals so the float value does not show up as 0.0011980000417...
            calib[IMU_KEYS[i]] = std::round(static_cast<double>(pNewCalibration.*fields[i]) * 1e6) / 1e6;
        }
    } catch (const ordered_json::exception& e) {
        REPORT_ERROR("CCubeConfigLoader: malformed calibration entry for ip '", entryIP, "' in '", pConfigPath, "': ", e.what());
        return false;
    }

    // Serialize first, then overwrite in place, so a failed dump can not truncate the file
    const std::string text = root.dump(2) + "\n";
    std::ofstream out(pConfigPath, std::ios::trunc);
    if (!out.is_open()) {
        REPORT_ERROR("CCubeConfigLoader: could not open '", pConfigPath, "' for writing");
        return false;
    }
    out << text;
    out.flush();
    if (!out.good()) {
        REPORT_ERROR("CCubeConfigLoader: failed to write '", pConfigPath, "'");
        return false;
    }

    if (pOutOldCalibration != nullptr) {
        *pOutOldCalibration = oldCalibration;
    }
    return true;
}
