#include <Config/util.h>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace yjcServer {
void FSUtil::ListAllFile(std::vector<std::string>& files,
                         const std::string&        path,
                         const std::string&        subfix) {
    try {
        auto absolute_path = fs::absolute(path);
        for (const auto& entry :
             fs::recursive_directory_iterator(absolute_path)) {
            if (entry.is_directory() &&
                (subfix.empty() ||
                 entry.path().filename().string().ends_with(subfix))) {
                files.push_back(entry.path().string());
            }
        }
    }
    catch (fs::filesystem_error& e) {
        spdlog::get("system_logger")
            ->error("ListAllFile() ,err: {}", e.what());
    }
}

}  // namespace yjcServer
