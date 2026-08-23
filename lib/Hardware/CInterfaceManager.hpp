#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <optional> 

template <typename InterfaceIdentifier, typename InterfaceType>
class CInterfaceManager{
public:
    CInterfaceManager(){}
    static std::optional<std::shared_ptr<InterfaceType>> getInstance(InterfaceIdentifier pId, bool pAllowMultipleInstances = false)
    {
        std::lock_guard<std::mutex> lock(mMtx);
        auto it = mInstances.find(pId);
        if (it != mInstances.end()) {
            if (pAllowMultipleInstances && it->second.second) {
                return it->second.first;
            }
            else {
                return std::nullopt;
            }
            
        }
        auto instance = std::shared_ptr<InterfaceType>(new InterfaceType(pId));
        mInstances[pId] = {instance, pAllowMultipleInstances};
    return instance;
    }
private:
    static inline std::map<InterfaceIdentifier, std::pair<std::shared_ptr<InterfaceType>, bool>> mInstances;
    static inline std::mutex mMtx;
};
