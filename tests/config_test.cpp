#include <Config/yjcServer.h>
#include <filesystem>

using namespace yjcServer;

ConfigVar<int>::ptr g_int_config =
    Config::Lookup("servers", std::string(""), "system.port");

int main() {
    Config::Visit([](std::shared_ptr<ConfigVarBase> v) {
        spdlog::info("name:{}, val:{}", v->getName(), v->toString());
    });
    Config::LoadFromConfigDir("/home/yjc/yjcServer/template/ymls/");
    Config::Visit([](std::shared_ptr<ConfigVarBase> v) {
        spdlog::info("name:{}, val:{}", v->getName(), v->toString());
    });

    return 0;
}