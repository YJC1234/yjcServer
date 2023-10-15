#pragma once

#include <pthread.h>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <thread>

namespace yjcServer {
class Thread {
public:
    using ptr = std::shared_ptr<Thread>;

    Thread(std::function<void()> cb, const std::string& name);
    Thread(std::function<void()> cb, const std::string&& rname);
    Thread(std::function<void()>& cb, const std::string& rname);
    Thread(std::function<void()>& cb, const std::string&& rname);

    ~Thread();

    pid_t getId() const {
        return m_id;
    }
    const std::string getName() const {
        return m_name;
    }

    void join();

    //线程内获取
    static Thread*     GetThis();
    static std::string GetName();
    static void        SetName(const std::string& name);

private:
    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(Thread&&) = delete;

    void init();  //减少重复代码
    void run();

private:
    pid_t                        m_id;
    std::unique_ptr<std::thread> m_thread;
    std::string                  m_name;
    std::function<void()>        m_cb;
};

}  // namespace yjcServer
