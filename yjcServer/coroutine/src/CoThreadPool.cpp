#include <coroutine/CoThreadPool.h>
#include <coroutine>

namespace yjcServer {

CoThreadPool::CoThreadPool() {
    m_pool = std::make_unique<ThreadPool>();
}

bool CoThreadPool::schedule_awaiter::await_ready() {
    return false;
}

void CoThreadPool::schedule_awaiter::await_suspend(
    std::coroutine_handle<> handle) {
    m_pool.push_task([&] { handle.resume(); });
}

CoThreadPool::schedule_awaiter CoThreadPool::schedule() {
    return schedule_awaiter{*m_pool};
}

};  // namespace yjcServer