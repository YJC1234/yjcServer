#include <Config/util.h>
#include <io/IOUring.h>

#define IO_URING_QUEUE_SIZE 2048  // TODO:配置
#define BUFFER_GROUP_ID 0

namespace yjcServer {

IOUring::IOUring() {
    int res = io_uring_queue_init(IO_URING_QUEUE_SIZE, m_ring, 0);
    YJC_ASSERT(res == 0);
}

IOUring::~IOUring() {
    io_uring_queue_exit(m_ring);
}

IOUring& IOUring::Instance() {
    thread_local IOUring ring;
    return ring;
}

io_uring* IOUring::get() const {
    return m_ring;
}

//-----------------------buf_ring-------------------------

void IOUring::setup_buf_ring(io_uring_buf_ring*           buf_ring,
                             std::span<std::vector<char>> buf_ring_list,
                             unsigned int                 buf_ring_size) {
    io_uring_buf_reg reg{.ring_addr = reinterpret_cast<uint64_t>(buf_ring),
                         .ring_entries = buf_ring_size,
                         .bgid = BUFFER_GROUP_ID};
    //将buf_ring注册到内核中
    const int result = io_uring_register_buf_ring(m_ring, &reg, 0);
    YJC_ASSERT(result == 0);

    //初始化环上的每个buffer
    const unsigned int mask = io_uring_buf_ring_mask(buf_ring_size);
    io_uring_buf_ring_init(buf_ring);
    for (unsigned int id = 0; id < buf_ring_size; ++id) {
        io_uring_buf_ring_add(buf_ring, buf_ring_list[id].data(),
                              buf_ring_list[id].size(), id, mask, id);
    }
    //移交给内核
    io_uring_buf_ring_advance(buf_ring, buf_ring_size);
}

void IOUring::add_buf(io_uring_buf_ring* buf_ring, std::span<char> buf,
                      const unsigned int buf_id,
                      const unsigned int buf_ring_size) {
    const unsigned int mask = io_uring_buf_ring_mask(buf_ring_size);
    io_uring_buf_ring_add(buf_ring, buf.data(),buf.size(), buf_id, mask, buf_id);
    io_uring_buf_ring_advance(buf_ring, 1);
}

}  // namespace yjcServer