#include <Config/yjcServer.h>
#include <filesystem>

using namespace yjcServer;

int main() {
    Config::Visit([](ConfigVarBase::ptr v) { spdlog::info("dinner"); });
    return 0;
}