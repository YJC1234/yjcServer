#include <Config/Config.h>
#include <spdlog/spdlog.h>
#include <thread/Thread.h>
#include <thread/th_helper.h>

namespace yjcServer {

//----------线程---------
static thread_local Thread*     t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOWN";

Thread* Thread::GetThis() {
    return t_thread;
}

std::string Thread::GetName() {
    return t_thread_name;
}

void Thread::SetName(const std::string& name) {
    if (name.empty()) {
        return;
    }
    if (t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}
//----构造------
Thread::Thread(std::function<void()> cb, const std::string& name)
    : m_cb(std::move(cb)),
      m_name(name.empty() ? "UNKNOWN" : std::move(name)) {
    init();
}
Thread::Thread(std::function<void()> cb, const std::string&& name)
    : m_cb(std::move(cb)), m_name(name.empty() ? "UNKNOWN" : name) {
    init();
}

Thread::Thread(std::function<void()>& cb, const std::string& name)
    : m_cb(std::move(cb)),
      m_name(name.empty() ? "UNKNOWN" : std::move(name)) {
    init();
}
Thread::Thread(std::function<void()>& cb, const std::string&& name)
    : m_cb(std::move(cb)), m_name(name.empty() ? "UNKNOWN" : name) {
    init();
}

void Thread::init() {
    try {
        m_thread = std::make_unique<std::thread>(&Thread::run, this);
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return m_started.load(); });
    }
    catch (const std::system_error& e) {
        spdlog::get("system_logger")
            ->error("thread create fail : {} ,name = {}", e.what(), m_name);
        throw;
    }
}

//---构造结束------

Thread::~Thread() {
    if (m_thread->joinable()) {
        m_thread->detach();
    }
}

void Thread::join() {
    if (m_thread->joinable()) {
        m_thread->join();
    } else {
        spdlog::get("system_logger")
            ->error("thread join fail ,name = {}.", m_name);
    }
}

void Thread::run() {
    t_thread = this;
    t_thread_name = m_name;
    m_id = GetThreadId();
    pthread_setname_np(pthread_self(), m_name.substr(0, 15).c_str());
    m_started = true;
    m_cv.notify_one();
    m_cb();
}

//--------自旋锁----------
void Spinlock::lock() {
    while (m_flag.test_and_set(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
}

void Spinlock::unlock() {
    m_flag.clear(std::memory_order_release);
}

}  // namespace yjcServer
