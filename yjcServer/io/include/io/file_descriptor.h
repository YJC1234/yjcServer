#pragma once
#include <io/IOUring.h>
#include <optional>

namespace yjcServer {

class file_descriptor {
private:
    std::optional<int> m_raw_fd;

public:
    file_descriptor();
    file_descriptor(const int raw_fd);
    ~file_descriptor();

    file_descriptor(file_descriptor&& other);
    file_descriptor& operator=(file_descriptor&& other);

    file_descriptor(const file_descriptor&) = delete;
    file_descriptor& operator=(const file_descriptor&) = delete;

    int get_raw_fd() const;

    class splice_awaiter {
        
    };
};

}  // namespace yjcServer
