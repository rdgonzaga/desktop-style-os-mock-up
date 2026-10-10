#include "core/Clock.h"

#include <ctime>

#include "Config.h"

namespace sysclock {

static std::tm getLocalTime() {
    std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    return local;
}

std::string timeStr() {
    std::tm local = getLocalTime();
    std::string format;
    if (config.clock24h) {
        format = config.clockSeconds ? "%H:%M:%S" : "%H:%M";
    } else {
        format = config.clockSeconds ? "%I:%M:%S %p" : "%I:%M %p";
    }
    char text[32];
    std::strftime(text, sizeof(text), format.c_str(), &local);
    return text;
}

std::string dateStr() {
    std::tm local = getLocalTime();
    char text[32];
    std::strftime(text, sizeof(text), "%m/%d/%Y", &local);
    return text;
}

std::string dateTime() {
    std::tm local = getLocalTime();
    char text[64];
    std::strftime(text, sizeof(text), "%A, %B %d, %Y", &local);
    return std::string(text) + " | " + timeStr();
}

}
