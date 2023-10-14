#include <logConfig/LogConfig.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace yjcServer {

void LogConfig::initSystemLogging() {
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "logs/system.txt", true);
    auto logger =
        std::make_shared<spdlog::logger>("system_logger", file_sink);
    spdlog::register_logger(logger);
}

}  // namespace yjcServer
