#include <Config/yjcServer.h>
#include <filesystem>

using namespace yjcServer;

auto g_logConfigInitializer = LogConfigInitializer::instance();

int main() {
    Config::Visit([](std::shared_ptr<ConfigVarBase> v) {
        spdlog::info(" name:{}\n val:{}", v->getName(), v->toString());
    });
    spdlog::info("after");
    Config::Visit([](std::shared_ptr<ConfigVarBase> v) {
        spdlog::info(" name:{}\n type:{}\n val:{}\n", v->getName(),
                     v->getTypeName(), v->toString());
    });

    return 0;
}