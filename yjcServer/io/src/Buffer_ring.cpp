#include <io/Buffer_ring.h>
#include <io/IOUring.h>
#include <spdlog/spdlog.h>
#include <unistd.h>
#include <cstdlib>

namespace yjcServer {

Buffer_ring& Buffer_ring::Instance() {
    thread_local Buffer_ring instance;
    return instance;
}

void Buffer_ring::register_buf_ring(const unsigned int buf_ring_size,
                                    const size_t       buf_size) {
    const size_t ring_entries_size = buf_ring_size * sizeof(io_uring_buf);
    const size_t page_alignment = sysconf(_SC_PAGESIZE);
    void*        buf_ring = nullptr;
    //申请环形队列的内存
    posix_memalign(&buf_ring, page_alignment, ring_entries_size);
    m_buf_ring.reset(reinterpret_cast<io_uring_buf_ring*>(buf_ring));
    m_buf_list.reserve(buf_ring_size);
    for (size_t i = 0; i < buf_ring_size; ++i) {
        m_buf_list.emplace_back(buf_size);
    }

    IOUring::Instance().setup_buf_ring(m_buf_ring.get(), m_buf_list, buf_size);
}

std::span<char> Buffer_ring::borrow_buf(const unsigned int buf_id,
                                        const size_t       size) {
    if (m_borrowed_buf_set[buf_id]) {
        spdlog::get("task_logger")
            ->error(
                "[Buffer_ring:borrow_buf]: the buf_id:{} is already borrow!",
                buf_id);
        return {};
    }
    m_borrowed_buf_set[buf_id] = true;
    return {m_buf_list[buf_id].data(), size};
}

void Buffer_ring::return_buf(const unsigned int buf_id) {
    m_borrowed_buf_set[buf_id] = false;
    //归还缓冲区
    IOUring::Instance().add_buf(m_buf_ring.get(), m_buf_list[buf_id], buf_id, m_buf_list.size());
}
}  // namespace yjcServer
