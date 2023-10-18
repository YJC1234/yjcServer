#include <Config/yjcServer.h>
#include <filesystem>

using namespace yjcServer;

ConfigVar<std::vector<int>>::ptr g_int_config =
    Config::Lookup<std::vector<int>>("servers.vec", std::vector<int>{1},
                                     "system.port");

int main() {
    Config::Visit([](std::shared_ptr<ConfigVarBase> v) {
        spdlog::info("name:{}, val:{}", v->getName(), v->toString());
    });
    Config::LoadFromConfigDir("/home/yjc/yjcServer/template/ymls/");
    Config::Visit([](std::shared_ptr<ConfigVarBase> v) {
        spdlog::info("name:{},type:{}, val:{}", v->getName(),
                     v->getTypeName(), v->toString());
    });

    return 0;
}