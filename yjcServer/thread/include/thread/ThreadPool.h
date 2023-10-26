#pragma once
#include <Config/common.h>
#include <spdlog/spdlog.h>
#include <thread/Thread.h>

namespace yjcServer {

class ThreadPool {
public:
    using Lock = std::unique_lock<std::mutex>;

private:
    std::vector<std::shared_ptr<yjcServer::Thread>> m_threads =
        {};                                          //工作线程
    std::queue<std::function<void()>> m_tasks = {};  //任务队列
    size_t m_running_tasks_count = 0;  //正在运行的任务数量
    size_t m_threads_count;            //工作线程数量
    bool   m_workers_running = false;  //线程池是否正在运行
    bool   m_waiting = false;          //线程池退出前等待
    std::condition_variable m_task_available_cv = {};
    std::condition_variable m_task_done_cv = {};
    std::mutex              m_mutex;

private:
    size_t determine_thread_count(size_t thread_count) {
        if (thread_count) {
            return thread_count;
        }
        size_t count = std::thread::hardware_concurrency();
        return count > 0 ? count : 0;
    }

    void worker() {
        std::function<void()> task;
        while (true) {
            Lock lock(m_mutex);
            m_task_available_cv.wait(lock, [this] {
                return !m_workers_running || !m_tasks.empty();
            });
            if (!m_workers_running) {
                return;
            }
            task = std::move(m_tasks.front());
            m_tasks.pop();
            ++m_running_tasks_count;
            lock.unlock();
            task();
            lock.lock();
            --m_running_tasks_count;
            if (m_waiting && !m_running_tasks_count && m_tasks.empty()) {
                m_task_done_cv.notify_all();
            }
        }
    }

    void wait_for_tasks() {
        Lock lock(m_mutex);
        m_waiting = true;
        m_task_done_cv.wait(lock, [this] {
            return !m_running_tasks_count && m_tasks.empty();
        });
        m_waiting = false;
    }
    void threads_destroy() {
        {
            Lock lock(m_mutex);
            m_workers_running = false;
        }
        m_task_available_cv.notify_all();
        for (size_t i = 0; i < m_threads_count; ++i) {
            m_threads[i]->join();
        }
    }

public:
    ThreadPool(size_t thread_count = 0)
        : m_threads_count(determine_thread_count(thread_count)) {
        {
            Lock lock(m_mutex);
            m_workers_running = true;
        }
        for (size_t i = 0; i < m_threads_count; ++i) {
            auto thread =
                std::make_shared<yjcServer::Thread>([this] { worker(); });
            m_threads.emplace_back(thread);
        }
        spdlog::get("task_logger")
            ->info("\nThreadPool启动! thread_count = {}\n;",
                   m_threads_count);
    }

    ~ThreadPool() {
        wait_for_tasks();
        threads_destroy();
    }

    template <class F, class... A>
    void push_task(F&& task, A&&... args) {
        {
            Lock lock(m_mutex);
            m_tasks.push(
                std::bind(std::forward<F>(task), std::forward<A>(args)...));
        }
        spdlog::get("task_logger")->info("[ThreadPool] new task add!");
        m_task_available_cv.notify_one();
    }

};  // class ThreadPool

}  // namespace yjcServer
