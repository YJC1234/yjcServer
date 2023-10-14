#include <logConfig/LogConfig.h>
#include <spdlog/spdlog.h>
#include <thread/Thread.h>
#include <thread/th_helper.h>

namespace yjcServer {

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
    : m_cb(cb), m_name(name) {
    if (m_name.empty()) {
        m_name = "UNKNOWN";
    }
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if (rt) {
        spdlog::get("system_logger")
            ->error("pthread_create fail, rt = {}, name = {}", rt, m_name);
        throw std::logic_error("pthread_create error.");
    }
}
Thread::Thread(std::function<void()> cb, const std::string&& rname)
    : m_cb(cb), m_name(rname) {
    if (m_name.empty()) {
        m_name = "UNKNOWN";
    }
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if (rt) {
        spdlog::get("system_logger")
            ->error("pthread_create fail, rt = {}, name = {}", rt, m_name);
        throw std::logic_error("pthread_create error.");
    }
}
//---构造结束------

Thread::~Thread() {
    if (m_thread) {
        pthread_detach(m_thread);
    }
}

void Thread::join() {
    if (m_thread) {
        int rt = pthread_join(m_thread, nullptr);
        if (rt) {
            spdlog::get("system_logger")
                ->error("pthread_join fail, rt = {}, name = {}", rt,
                        m_name);
            throw std::logic_error("pthread_join error.");
        }
        m_thread = 0;
    }
}

void* Thread::run(void* arg) {
    Thread* thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->m_name;
    // pthread_create只会写pthread_t，对于pid_t需要自己再传
    thread->m_id = GetThreadId();
    pthread_setname_np(pthread_self(),
                       thread->m_name.substr(0, 15).c_str());

    std::function<void()> cb;
    cb.swap(thread->m_cb);

    cb();
    return 0;
}
}  // namespace yjcServer
