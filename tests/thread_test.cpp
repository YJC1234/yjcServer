#include <logConfig/yjcServer.h>
#include <iostream>
#include <vector>

void fun1() {
    spdlog::info("name:{},this.name{}; id:{};",
                 yjcServer::Thread::GetName(),
                 yjcServer::Thread::GetThis()->getName(),
                 yjcServer::Thread::GetThis()->getId());
}

void fun2() {}

int main() {
    std::vector<yjcServer::Thread::ptr> thrs;
    for (int i = 0; i < 5; i++) {
        yjcServer::Thread::ptr thr(
            new yjcServer::Thread(fun1, "fun" + std::to_string(i)));
        thrs.push_back(std::move(thr));
    }
    for (int i = 0; i < 5; i++) {
        thrs[i]->join();
    }
}