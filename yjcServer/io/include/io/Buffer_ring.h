#pragma once
#include <liburing.h>
#include <bitset>
#include <memory>
#include <span>
#include <vector>

#define MAX_BUFFER_RING_SIZE 65536

namespace yjcServer {
//环形缓冲区
class Buffer_ring {
private:
    std::unique_ptr<io_uring_buf_ring> m_buf_ring;
    std::vector<std::vector<char>>     m_buf_list;
    std::bitset<MAX_BUFFER_RING_SIZE>  m_borrowed_buf_set;

public:
    /// @brief 线程单例
    static Buffer_ring& Instance();

    /// @brief 注册缓冲区
    /// @param buf_ring_size 分配的io_uring_buf个数
    /// @param buf_size m_buf_list中每个buf的大小
    void register_buf_ring(const unsigned int buf_ring_size,
                           const size_t       buf_size);

    /// @brief 借用缓冲区
    /// @param buf_id 要借用的缓冲区id
    /// @param size 缓冲区大小
    /// @return buf指针和size的span，如果已经被借用返回null
    std::span<char> borrow_buf(const unsigned int buf_id, const size_t size);

    /// @brief 归还缓冲区，释放资源
    /// @param buffer_id 归还的buf id
    void return_buf(const unsigned int buf_id);
};
}  // namespace yjcServer