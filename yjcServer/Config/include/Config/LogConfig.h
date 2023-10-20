#pragma once
#include <Config/Config.h>
#include <map>
#include <string>

namespace yjcServer {

class LogConfigInitializer {
public:
    /// @brief 单例，只初始化一次
    static LogConfigInitializer& instance();

private:
    LogConfigInitializer();

    void init();
};

}  // namespace yjcServer
