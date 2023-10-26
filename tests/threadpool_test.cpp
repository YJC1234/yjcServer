#include <Config/yjcServer.h>
#include <thread/ThreadPool.h>

using namespace yjcServer;

long long         count = 0;
std::shared_mutex rwmutex;

void fun1() {
    spdlog::info("name:{},this.name{}; id:{};",
                 yjcServer::Thread::GetName(),
                 yjcServer::Thread::GetThis()->getName(),
                 yjcServer::Thread::GetThis()->getId());
    std::unique_lock<std::shared_mutex> lock(rwmutex);
    for (int i = 0; i < 1000000; i++) {
        count = count + 1;
    }
}

int main() {
    LogConfigInitializer::instance();
    ThreadPool* pool = new ThreadPool;
    for (size_t i = 0; i < 5; i++) {
        pool->push_task([] { fun1(); });
    }
    delete pool;
    spdlog::info("count = {}.", count);
}