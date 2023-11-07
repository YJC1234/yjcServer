#include <Config/yjcServer.h>
#include <filesystem>

using namespace yjcServer;

int main() {
    LogConfigInitializer::instance();
    spdlog::get("system_logger")->info("dinner");
    spdlog::get("task_logger")->info("dinner");
    return 0;
}