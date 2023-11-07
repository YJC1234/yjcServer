#include <Config/util.h>
#include <io/IOUring.h>
#include <spdlog/spdlog.h>

#define IO_URING_QUEUE_SIZE 2048  // TODO:配置

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
io_uring_buf_ring* IOUring::setup_buf_ring(unsigned int nentries) {
    io_uring_buf_ring* br;
}

}  // namespace yjcServer