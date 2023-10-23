#pragma once
#include <spdlog/spdlog.h>
#include <cassert>
#include <source_location>
#include <string>
#include <vector>

namespace yjcServer {

/// @brief 文件系统相关的一些封装
class FSUtil {
public:
    /// @brief 递归将path路径文件夹中所有subfix后缀的文件存入files
    /// @param files 存放的vector
    /// @param path 文件夹路径
    /// @param subfix 后缀
    static void ListAllFile(std::vector<std::string>& files,
                            const std::string&        path,
                            const std::string&        subfix);
};

//----------------------------------断言宏------------------------------------------
namespace details {
inline void logError(const std::source_location& loc,
                     const std::string&          msg) {
    spdlog::get("system_logger")
        ->error("Error at:\n filename: {}\nline : {}\nfun : {}\n msg: {}",
                loc.file_name(), loc.line(), loc.function_name(), msg);
}

inline void assertWithLog(
    bool condition, const std::string& msg,
    const std::source_location loc = std::source_location::current()) {
    if (!condition) [[unlikely]] {
        logError(loc, msg);
        assert(condition);
    }
}

}  // namespace details

#define YJC_ASSERT(x) yjcServer::details::assertWithLog(x, "ASSERTION: " #x)

#define YJC_ASSERT_MSG(x, msg)                                             \
    yjcServer::details::assertWithLog(x, "ASSERTION: " #x "\n" msg)

}  // namespace yjcServer