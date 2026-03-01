#include <iostream>
#include <string>
#include <sstream>
#include <errno.h>

class ErrorReporter {
private:
    template<typename T>
    static void appendToStream(std::ostringstream& oss, const T& value) {
        oss << value;
    }

    template<typename T, typename... Args>
    static void appendToStream(std::ostringstream& oss, const T& first, const Args&... rest) {
        oss << first;
        appendToStream(oss, rest...);
    }

    template<typename... Args>
    static std::string buildMessage(const Args&... args) {
        std::ostringstream oss;
        appendToStream(oss, args...);
        return oss.str();
    }

public:
    template<typename... Args>
    static void logError(const char* file, int line, const char* function, const Args&... args) {
        std::cerr << "[ERROR] " << file << ":" << line << " (" << function << "): " 
                  << buildMessage(args...) << '\n';
    }

    template<typename... Args>
    static void logErrorErrno(const char* file, int line, const char* function, const Args&... args) {
        int err = errno;
        std::cerr << "[ERROR] " << file << ":" << line << " (" << function << "): " 
                  << buildMessage(args...) << ", errno: " << err << '\n';
    }
};

#define REPORT_ERROR(...) ErrorReporter::logError(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define REPORT_ERROR_ERRNO(...) ErrorReporter::logErrorErrno(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
