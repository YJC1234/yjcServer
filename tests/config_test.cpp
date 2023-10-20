#include <Config/yjcServer.h>
#include <filesystem>

using namespace yjcServer;

int main() {
    LogConfigInitializer::instance();
    Config::Visit(
        [](ConfigVarBase::ptr v) { spdlog::info("\n{}", v->getName()); });
    return 0;
}