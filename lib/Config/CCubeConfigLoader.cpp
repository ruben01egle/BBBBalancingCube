#include "CCubeConfigLoader.hpp"

#include <climits>
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

    if (!root.contains("cubes") || !root["cubes"].is_array()) {
        REPORT_ERROR("CCubeConfigLoader: cube config file '", pConfigPath, "' is missing a 'cubes' array");
        return false;
    }

    for (const auto& entry : root["cubes"]) {
        std::string entryIP;
        try {
            entryIP = entry.at("ip").get<std::string>();
        } catch (const json::exception& e) {
            REPORT_ERROR("CCubeConfigLoader: malformed entry in '", pConfigPath, "' (missing/invalid 'ip'): ", e.what());
            return false;
        }

        bool matches = false;
        for (const auto& localIP : localIPs) {
            if (localIP == entryIP) {
                matches = true;
                break;
            }
        }
        if (!matches) {
            continue;
        }

        try {
            pOutCubeId = entry.at("cube").get<int>();
            const auto& calib = entry.at("calibration");
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

    std::string localIPList;
    for (size_t i = 0; i < localIPs.size(); ++i) {
        if (i > 0) localIPList += ", ";
        localIPList += localIPs[i];
    }
    REPORT_ERROR("CCubeConfigLoader: none of this host's local IPs [", localIPList, "] match any entry in '", pConfigPath, "'");
    return false;
}
