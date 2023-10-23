#include <Config/yjcServer.h>

using namespace yjcServer;

int main() {
    LogConfigInitializer::instance();
    int x = 0;
    YJC_ASSERT(x == 1);
}