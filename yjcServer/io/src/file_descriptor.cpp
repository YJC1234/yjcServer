#include <io/file_descriptor.h>
#include <utility>

namespace yjcServer {
file_descriptor::file_descriptor() = default;

file_descriptor::file_descriptor(const int raw_fd) : m_raw_fd(raw_fd) {}

file_descriptor::~file_descriptor() {
    if (m_raw_fd.has_value()) {
        close(m_raw_fd.value());
    }
}

file_descriptor::file_descriptor(file_descriptor&& other)
    : m_raw_fd(other.m_raw_fd) {
    other.m_raw_fd = std::nullopt;
}

file_descriptor& file_descriptor::operator=(file_descriptor&& other) {
    if (this == std::addressof(other)) {
        return *this;
    }
    m_raw_fd = std::exchange(other.m_raw_fd, std::nullopt);
    return *this;
}

int file_descriptor::get_raw_fd() const {
    return m_raw_fd.value();
}
}  // namespace yjcServer
