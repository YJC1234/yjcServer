#pragma once

#include <Config/util.h>
#include <concepts>
#include <coroutine>
#include <exception>
#include <memory>

#define YJC_CO_AWAIT_HINT nodiscard("Did you forget to co_await?")

/*
 *协程task的封装
 *协程定义 task<> coFun() { xxxx },task<>作为协程的返回值
 * task为惰性,auto i = coFun(); 此时协程不会执行
 *必须使用co_await i/co_await coFun()才可执行协程。
 *协程中co_await其他协程，会进行切换，其他协程执行完后会回到本协程/父协程
 *协程可以使用co_return结束，并将co_return之后的结果放在promise中
 *使用auto res = co_await coFun()可以获得协程的结果
 */

namespace yjcServer {

template <class T>
class task;

template <class T>
class task_promise_base;

//----------------------struct task_final_awaiter--------------------------

/// @brief 子协程结束时恢复其父协程
template <class T>
struct task_final_awaiter {
    //结束时总是跳转
    static constexpr bool await_ready() noexcept {
        return false;
    }

    template <std::derived_from<task_promise_base<T>> Promise>
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<Promise> current) noexcept {
        return current.promise().m_parent_coro;
    }
    //不会resume
    void await_resume() noexcept {}
};

template <>
struct task_final_awaiter<void> {
    //结束时总是跳转
    static constexpr bool await_ready() noexcept {
        return false;
    }

    //子协程如果分离，则销毁
    template <std::derived_from<task_promise_base<void>> Promise>
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<Promise> current) noexcept {
        auto&                   promise = current.promise();
        std::coroutine_handle<> parent = promise.m_parent_coro;
        if (promise.m_is_detached_flag == Promise::is_detached) {
            current.destroy();
        }
        return parent;
    }
    //不会resume
    void await_resume() noexcept {}
};
//-------------------------------------------------------------------------

//-----------------------class task_promise_base--------------------------

/// final_suspend: yes, and return to parent.
template <class T>
class task_promise_base {
    friend struct task_final_awaiter<T>;

private:
    std::coroutine_handle<> m_parent_coro{std::noop_coroutine()};

public:
    task_promise_base() noexcept = default;

    //惰性协程，创建时不会立即执行
    std::suspend_always initial_suspend() noexcept {
        return {};
    }

    task_final_awaiter<T> final_suspend() noexcept {
        return {};
    }

    void set_parent(std::coroutine_handle<> parent) {
        m_parent_coro = parent;
    }

    //禁止拷贝/移动
    task_promise_base(const task_promise_base&) = delete;
    task_promise_base(task_promise_base&&) = delete;
    task_promise_base& operator=(const task_promise_base&) = delete;
    task_promise_base& operator=(task_promise_base&&) = delete;
};
//-------------------------------------------------------------------------
//------------------------class task_promise-------------------------------
template <class T>
class task_promise final : public task_promise_base<T> {
private:
    union {
        T                  m_value;
        std::exception_ptr m_exception_ptr;
    };
    enum class value_state : uint8_t { init, value, exception } m_state;

public:
    task_promise() noexcept : m_state(value_state::init) {}

    // union必须手动选择析构
    ~task_promise() noexcept {
        switch (m_state) {
            [[likely]] case value_state::value : value.~T();
            break;
        case value_state::exception:
            exception_ptr.~exception_ptr();
            break;
        default:
            break;
        }
    }

    /// @brief 需要放在task类定义的后面
    /// @return task<T>{std::co.._handle<task_promise>::from_promise(*this)}
    task<T> get_return_object() noexcept;

    void unhandled_exception() noexcept {
        m_exception_ptr = std::current_exception();
        m_state = value_state::exception;
    }

    template <typename Value>
    requires std::convertible_to<Value&&, T>
    void return_value(Value&& result) noexcept(
        std::is_nothrow_constructible_v<T, Value&&>) {
        std::construct_at(std::addressof(value),
                          std::forward<Value>(result));  //原地构造，union需要
        m_state = value_state::value;
    }

    // get the lvalue ref
    T& result() & {
        if (m_state == value_state::exception) [[unlikely]] {
            std::rethrow_exception(m_exception_ptr);
        }
        YJC_ASSERT(m_state == value_state::value);
        return m_value;
    }

    // get the prvalue
    T&& result() && {
        if (m_state == value_state::exception) [[unlikely]] {
            std::rethrow_exception(m_exception_ptr);
        }
        YJC_ASSERT(m_state == value_state::value);
        return std::move(m_value);
    }
};

template <>
class task_promise<void> final : public task_promise_base<void> {
    friend struct task_final_awaiter<void>;
    friend class task<void>;

private:
    static constexpr uintptr_t is_detached = -1ULL;

    union {
        uintptr_t m_is_detached_flag;  // set to `is_detached` if is detached.
        std::exception_ptr m_exception_ptr;
    };

public:
    task_promise() noexcept : m_is_detached_flag(0){};

    ~task_promise() noexcept {
        if (m_is_detached_flag != is_detached) {
            m_exception_ptr.~exception_ptr();
        }
    }

    task<void> get_return_object() noexcept;

    constexpr void return_void() noexcept {}

    void unhandled_exception() {
        if (m_is_detached_flag == is_detached) {
            std::rethrow_exception(std::current_exception());
        } else {
            m_exception_ptr = std::current_exception();
        }
    }

    void result() const {
        if (this->m_exception_ptr) [[unlikely]] {
            std::rethrow_exception(this->m_exception_ptr);
        }
    }
};

template <typename T>
class task_promise<T&> final : public task_promise_base<T&> {
private:
    T*                 m_value = nullptr;
    std::exception_ptr m_exception_ptr;

public:
    task_promise() noexcept = default;

    task<T&> get_return_object() noexcept;

    void unhandled_exception() noexcept {
        this->m_exception_ptr = std::current_exception();
    }

    void return_value(T& result) noexcept {
        m_value = std::addressof(result);
    }

    T& result() {
        if (m_exception_ptr) [[unlikely]] {
            std::rethrow_exception(m_exception_ptr);
        }
        return *m_value;
    }
};

//----------------------------------------------------------------------------

//----------------------------class task--------------------------------------
template <class T = void>
class [[YJC_CO_AWAIT_HINT]] task {
public:
    using promise_type = task_promise<T>;
    using value_type = T;

private:
    std::coroutine_handle<promise_type> m_handle;

private:
    struct awaiter_base {
        std::coroutine_handle<promise_type> handle;

        explicit await_base(std::coroutine_handle<promise_type> current)
            : handle(current) {}

        bool await_ready() const noexcept {
            return !handle || handle.done();
        }

        //将当前协程设置为此协程的父协程，然后切换到此协程
        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<> awaiting_coro) noexcept {
            handle.promise().set_parent(awaiting_coro);
            return handle;
        }
    };

public:
    task() = default;
    explicit task(std::coroutine_handle<promise_type> current) noexcept
        : m_handle(current) {}

    task(task&& other) noexcept : m_handle(other.handle) {
        other.handle = nullptr;
    }

    // Ban copy
    task(const task<>&) = delete;
    task& operator=(const task&) = delete;

    task& operator=(task&& other) noexcept {
        if (this != std::addressof(other)) [[likely]] {
            if (handle) {
                m_handle.destroy();
            }
            handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    // Free the promise object and coroutine parameters
    ~task() {
        if (handle) {
            m_handle.destroy();
        }
    }

    [[nodiscard]] bool is_ready() const noexcept {
        return !handle || handle.done();
    }

    /// @brief 使用co_await task<>切换到此协程中，在协程结束后回来并返回ref
    auto operator co_await() const& noexcept {
        struct awaiter : awaiter_base {
            decltype(auto) await_resume() {
                YJC_ASSERT(handle);
                return handle.promise().result();
            }
        };
        return awaiter{m_handle};
    }

    /// @brief 使用co_await task<>切换到此协程中，在协程结束后回来并返回rvalue
    /// ref
    auto operator co_await() const&& noexcept {
        struct awaiter : awaiter_base {
            decltype(auto) await_resume() {
                YJC_ASSERT(handle);
                return std::move(handle.promise()).result();
            }
        };
        return awaiter{m_handle};
    }

    ///@brief 切换到协程中，但是不返回结果
    [[nodiscard]] auto when_ready() const noexcept {
        struct awaiter : awaiter_base {
            using awaiter_base::awaiter_base;

            constexpr void await_resume() const noexcept {}
        };

        return awaiter{m_handle};
    }

    std::coroutine_handle<promise_type> get_handle() noexcept {
        return handle;
    }

    void detach() noexcept {
        if constexpr (std::is_void_v<value_type>) {
            m_handle.promise().m_is_detached_flag = promise_type::is_detached;
        }
        m_handle = nullptr;
    }

    friend void swap(task& a, task& b) noexcept {
        std::swap(a.m_handle, b.m_handle);
    }
};

template <typename T>
inline task<T> task_promise<T>::get_return_object() noexcept {
    return task<T>{std::coroutine_handle<task_promise>::from_promise(*this)};
}

inline task<void> task_promise<void>::get_return_object() noexcept {
    return task<void>{std::coroutine_handle<task_promise>::from_promise(*this)};
}

template <typename T>
inline task<T&> task_promise<T&>::get_return_object() noexcept {
    return task<T&>{std::coroutine_handle<task_promise>::from_promise(*this)};
}

}  // namespace yjcServer