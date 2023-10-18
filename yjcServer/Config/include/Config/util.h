#pragma once
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

}  // namespace yjcServer