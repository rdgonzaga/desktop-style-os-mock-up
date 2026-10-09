#include "core/Clock.h"

#include <ctime>

#include "Config.h"

namespace sysclock {

std::string dateTime() {
    std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif

    std::string format = "%A, %b %d, %Y | ";
    if (config.clock24h) {
        format += config.clockSeconds ? "%H:%M:%S" : "%H:%M";
    } else {
        format += config.clockSeconds ? "%I:%M:%S %p" : "%I:%M %p";
    }

    char text[64];
    std::strftime(text, sizeof(text), format.c_str(), &local);
    return text;
}

}
