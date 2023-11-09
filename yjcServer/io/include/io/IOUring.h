#pragma once
#include <liburing.h>
#include <span>
#include <vector>

namespace yjcServer {

struct SqeData {
    void*        handle = 0;
    int          cqe_res = 0;
    unsigned int cqe_flag = 0;
};

class IOUring {
private:
    struct io_uring* m_ring;

    IOUring();
    ~IOUring();

public:
    IOUring(IOUring&& other) = delete;
    IOUring& operator=(IOUring&& other) = delete;
    IOUring(const IOUring& other) = delete;
    IOUring& operator=(const IOUring& other) = delete;

    /// @brief 获取IOUirng的单例对象
    static IOUring& Instance();
    /// @brief 获取原始的io_uring
    io_uring* get() const;

    /// @brief 内核注册io_uring_buf_ring，用于提供缓冲区
    /// @param buf_ring 要注册的缓冲区
    /// @param buf_ring_list
    /// @param buf_ring_size
    /// @return
    void setup_buf_ring(io_uring_buf_ring*           buf_ring,
                        std::span<std::vector<char>> buf_ring_list,
                        unsigned int                 buf_ring_size);
    /// @brief 将buf_id对应的缓冲区归还给内核
    /// @param buf_ring buf_ring实例
    /// @param buf 要归还的缓冲区实例
    /// @param buf_id 要归还的缓冲区id
    /// @param buf_ring_size 缓冲区个数
    void add_buf(io_uring_buf_ring* buf_ring, std::span<char> buf,
                 const unsigned int buf_id, const unsigned int buf_ring_size);
};
}  // namespace yjcServer
