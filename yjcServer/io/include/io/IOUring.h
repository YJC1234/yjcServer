#pragma once
#include <liburing.h>

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

    /// @brief 注册一个共享的缓冲区环，用于提供。可以避免每次提交任务前都必须手动分配缓冲区，如果大量任务会导致内存紧张的问题。
    /// @note  如果请求准备接收数据，并且在SQE 标志中设置了 IOSQE_BUFFER_SELECT，则选择一个缓冲区。
    /// @param nentries 缓冲区环中请求的条目数
    /// @return 如果注册成功，返回io_uring_buf_ring*，否则返回nullptr
    io_uring_buf_ring* setup_buf_ring(unsigned int nentries = 64);

};
}  // namespace yjcServer
