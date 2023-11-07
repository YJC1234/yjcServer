#include <io/Buffer_ring.h>
#include <spdlog.h>
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
    posix_memalign(&buf_ring, page_alignment, ring_entries_size);
    m_buf_ring.reset(reinterpret_cast<io_uring_buf_ring*>(buf_ring));
    m_buf_list.reserve(buf_ring_size);
    for (size_t i = 0; i < buf_ring_size; ++i) {
        m_buf_list.emplace_back(buf_size);
    }

    // TODO:在io_uring中注册Buffer_ring
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
    // TODO:将m_buf_list[buf_id]归还给内核的io_uring_buf_ring
}
}  // namespace yjcServer
