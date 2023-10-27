#pragma once
#include <common.h>
#include <thread/ThreadPool.h>
#include <coroutine>

namespace yjcServer {
class CoThreadPool {
private:
    std::unique_ptr<ThreadPool> m_pool;

public:
    CoThreadPool();

    class schedule_awaiter {
    private:
        ThreadPool& m_pool;

    public:
        schedule_awaiter(ThreadPool& pool) : m_pool(pool) {}

        bool await_ready();  // false
        void await_resume() {}
        //暂停，执行后跳转到调用者
        void await_suspend(std::coroutine_handle<> handle);

    };  // class schedule_awaiter

    schedule_awaiter schedule();
};
}  // namespace yjcServer