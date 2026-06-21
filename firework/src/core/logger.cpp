#include "logger.hpp"

namespace Firework
{
 
Logger::Logger(const std::string &directory, const std::string &subDirectory)
    : _directory{directory}
    , _subDirectory{subDirectory}
{}

auto Logger::get_time_string() -> std::string {
    using namespace std::chrono;

    system_clock::time_point now = system_clock::now();
    // Time zone offset
    seconds offset = current_zone()->get_info(now).offset;
    duration sysTime = now.time_since_epoch() + offset;

    int64_t s = duration_cast<seconds>(sysTime).count() % 60;
    int64_t m = duration_cast<minutes>(sysTime).count() % 60;
    int64_t h = duration_cast<hours>(sysTime).count()   % 24;

    return std::format("{:02}:{:02}:{:02}", h, m, s);
}

} // namespace Firework
