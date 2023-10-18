#include <Config/Config.h>
#include <Config/util.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <list>

namespace yjcServer {

int def_initSystemLogger = (LogConfig::initSystemLogger(), 0);

void LogConfig::initSystemLogger() {
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "logs/system.txt", true);
    auto logger =
        std::make_shared<spdlog::logger>("system_logger", file_sink);
    spdlog::register_logger(logger);
}

/*
 * --------------------------------------------------
 *-------------------- 配置模块-----------------------
 * --------------------------------------------------
 */

ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
    std::shared_lock<std::shared_mutex> lock(GetMutex());

    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

/// @brief 遍历一个节点，将节点和所有成员放入一个列表中
/// @param prefix 名称，格式为父节点.子节点
/// @param node 节点，可能是标量，序列，映射
/// @param output 输出列表，包含节点名称以及本身
static void
listAllMember(const std::string& prefix, const YAML::Node& node,
              std::list<std::pair<std::string, const YAML::Node>>& output) {
    if (prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") !=
        std::string::npos) {
        spdlog::get("system_logger")
            ->error("Config::listAllMember():Config name invalid:{} ",
                    prefix);
        return;
    }
    output.push_back({prefix, node});
    if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            listAllMember(prefix.empty()
                              ? it->first.Scalar()
                              : prefix + "." + it->first.Scalar(),
                          it->second, output);
        }
    }
}

/// @brief 从yaml导入到配置模块(注意只是导入值，名称导入前已经存在)
/// @param root
void Config::LoadFromYaml(const YAML::Node& root) {
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    listAllMember("", root, all_nodes);

    for (auto& i : all_nodes) {
        std::string key = i.first;
        if (key.empty()) {
            continue;
        }
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigVarBase::ptr var = LookupBase(key);
        if (!var) {
            continue;
        }
        //这里其实区分意义不大，无论标量与否,fromString都能够解析(已经实现了string与容器间的类型转换)
        if (i.second.IsScalar()) {
            var->fromString(i.second.Scalar());
        } else {
            std::stringstream ss;
            ss << i.second;
            var->fromString(ss.str());
        }
    }
}

/// @brief 加载path文件夹中所有配置文件
void Config::LoadFromConfigDir(const std::string& path,
                               bool               force = false) {
    std::vector<std::string> files;
    // TODO:这里直接将path转化为绝对路径，可能要修改
    FSUtil::ListAllFile(files, path, ".yml");

    for (auto& i : files) {
        // TODO:time
        try {
            YAML::Node root = YAML::LoadFile(i);
            LoadFromYaml(root);
            spdlog::get("system_logger")
                ->info("Load ConfigFile = {} OK.", i);
        }
        catch (...) {
            spdlog::get("system_logger")
                ->error("Load ConfigFile = {} failed.", i);
        }
    }
}

void Config::Visit(std::function<void(std::shared_ptr<ConfigVarBase>)> cb) {
    std::shared_lock<std::shared_mutex> lock(GetMutex());

    ConfigVarMap& m = GetDatas();
    for (auto it = m.begin(); it != m.end(); ++it) {
        cb(it->second);
    }
}

}  // namespace yjcServer
