#pragma once
#include <Config/common.h>
#include <atomic>
#include <coroutine>
#include <optional>
#include <type_traits>
#include <utility>

namespace yjcServer {

template <class T>
class task_promise;

template <class T = void>
class task {
public:
    using promise_type = task_promise<T>;

private:
    std::coroutine_handle<task_promise<T>> m_handle = nullptr;

public:
    explicit task(std::coroutine_handle<task_promise<T>> handle)
        : m_handle(handle) {}

    ~task() {
        if (m_handle) {
            if (m_handle.done()) {
                m_handle.destroy();
            } else {
                m_handle.promise().get_detached_flag().test_and_set(
                    std::memory_order_relaxed);
            }
        }
    }

    task(const task&& other)
        : m_handle(std::exchange(other.m_handle, nullptr)) {}

    task(const task& other) = delete;
    task operator=(const task& other) = delete;
    task operator=(const task&& other) = delete;

    class task_awaiter {  // co_await之后的对象
    private:
        //操作task协程，由于类内不能访问类外的private成员，因此设置
        std::coroutine_handle<task_promise<T>> m_handle;

    public:
        explicit task_awaiter(std::coroutine_handle<task_promise<T>> handle)
            : m_handle{handle} {}

        bool await_ready() const {
            return m_handle == nullptr || m_handle.done();
        }

        decltype(auto) await_resume() const noexcept {
            if constexpr (!std::is_same_v<T, void>) {
                return m_handle.promise().get_return_value();
            }
        }

        //遇到co_await task时不跳转，等到协程结束时跳转
        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<task_promise<T>> handle) const {
            m_handle.promise().get_calling_handle() = handle;
            return m_handle;
        }

    };  // class task_awaiter

    /// @brief 重载task的co_await,这样可以co_awiat
    /// task,但是具体suspend等逻辑放在task_awaiter中处理
    task_awaiter operator co_await() {
        return task_awaiter(m_handle);
    }

};  // class task

template <class T>
class task_promise_base {
private:
    std::optional<std::coroutine_handle<>> m_calling_handle;
    std::atomic_flag                       m_detach_flag;

public:
    class final_waiter {  //在协程的最后控制操作(通过final_suspend指定)
    public:
        bool await_ready() const {
            return false;
        }
        void await_resume() {}

        //如果flag，协程结束，否则跳转到m_calling_handle协程
        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<task_promise<T>> handle) {
            if (handle.promise().get_detached_flag().test(
                    std::memory_order_relaxed)) {
                handle.destroy();
            }
            return handle.promise().get_calling_handle().value_for(
                std::noop_coroutine());
        }
    };  // class final_waiter

    std::suspend_always initial_suspend() noexcept {  //协程开始总是暂停
        return {};
    }

    final_waiter final_suspend() noexcept {  //协程结束时由final_waiter控制
        return final_waiter{};
    }

    void unhandled_exception() const noexcept {
        return std::terminate();
    }

    std::optional<std::coroutine_handle<>>& get_calling_handle() {
        return m_calling_handle;
    }

    std::atomic_flag& get_detach_flag() {
        return m_detach_flag;
    }

};  // class taks_promise_base

template <class T>
class task_promise final : public task_promise_base<T> {
private:
    std::optional<T> m_return_value;

public:
    task<T> get_return_object() noexcept {
        return task<T>{
            std::coroutine_handle<task_promise<T>>::from_promise(*this)};
    }

    template <typename U>
    requires std::convertible_to<U&&, T>
    void return_value(U&& return_value) noexcept(
        std::is_nothrow_constructible_v<T, U&&>) {
        m_return_value.emplace(std::forward<U>(return_value));
    }

    T& get_return_value() & noexcept {
        return *m_return_value;
    }
    T&& get_return_value() && noexcept {
        return std::move(*m_return_value);
    }
};  // class task_promise

template <>
class task_promise<void> final : public task_promise_base<void> {
public:
    task<void> get_return_object() noexcept {
        return task<void>{
            std::coroutine_handle<task_promise>::from_promise(*this)};
    };

    void return_void() const noexcept {}
};  // class task_promise<>

}  // namespace yjcServer
