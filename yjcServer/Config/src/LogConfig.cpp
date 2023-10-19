#include <Config/LogConfig.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace yjcServer {

namespace fs = std::filesystem;

//执行初始化注册工作，单例设计
inline auto g_logConfigInitializer = LogConfigInitializer::instance();

/// @brief 定义sink的配置结构
struct SinkConfig {
    std::string type;
    std::string filename;
    size_t      max_files;
    size_t      max_size;
    std::string pattern;

    bool operator==(const SinkConfig& other) const {
        return type == other.type && filename == other.filename &&
               max_size == other.max_size && max_files == other.max_files &&
               pattern == other.pattern;
    }
};

/// @brief 定义Logger的配置结构
struct LoggerConfig {
    std::string             name;
    std::string             level;
    std::vector<SinkConfig> sinks;

    bool operator==(const LoggerConfig& other) const {
        return name == other.name && level == other.level &&
               sinks == other.sinks;
    }
};

/// @brief 定义全局的配置结构
struct GlobalConfig {
    bool   async;
    size_t thread_pool_size;

    bool operator==(const GlobalConfig& other) const {
        return async == other.async &&
               thread_pool_size == other.thread_pool_size;
    }
};

/*
 * -------------------------------------------------------------------------------------
 * -----------------------------解析自定义类型需要的类型转换---------------------------------
 * -------------------------------------------------------------------------------------
 */

/// @brief
/// fromString(SinkConfig),对于自定义类型，需要实现类型转换以便支持yaml的解析
template <>
class LexicalCast<std::string, SinkConfig> {
public:
    SinkConfig operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        SinkConfig res;
        if (node["type"]) {
            res.type = node["type"].as<std::string>();
        }
        if (node["filename"]) {
            res.filename = node["filename"].as<std::string>();
        }
        if (node["max_files"]) {
            res.max_files = node["max_files"].as<size_t>();
        }
        if (node["max_size"]) {
            res.max_size = node["max_size"].as<size_t>();
        }
        if (node["pattern"]) {
            res.pattern = node["pattern"].as<std::string>();
        }
        return res;
    }
};

/// @brief toString(SinkConfig)
template <>
class LexicalCast<SinkConfig, std::string> {
public:
    std::string operator()(const SinkConfig& v) {
        YAML::Node node;
        node["type"] = v.type;
        node["filename"] = v.filename;
        node["max_files"] = v.max_files;
        node["max_size"] = v.max_size;
        node["pattern"] = v.pattern;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/// @brief fromstring(LoggerConfig)
template <>
class LexicalCast<std::string, LoggerConfig> {
public:
    LoggerConfig operator()(const std::string& v) {
        YAML::Node   node = YAML::Load(v);
        LoggerConfig res;
        res.name = node["name"].as<std::string>();
        res.level = node["level"].as<std::string>();
        std::stringstream ss;
        ss << node["sinks"];
        res.sinks =
            LexicalCast<std::string, std::vector<SinkConfig>>()(ss.str());
        return res;
    }
};

/// @brief toString(LoggerConfig)
template <>
class LexicalCast<LoggerConfig, std::string> {
public:
    std::string operator()(const LoggerConfig& v) {
        YAML::Node node;
        node["name"] = v.name;
        node["level"] = v.level;
        node["sinks"] =
            LexicalCast<std::vector<SinkConfig>, std::string>()(v.sinks);
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/// @brief fromStirng(GlobalConfig)
template <>
class LexicalCast<std::string, GlobalConfig> {
public:
    GlobalConfig operator()(const std::string& v) {
        YAML::Node   node = YAML::Load(v);
        GlobalConfig res;
        res.async = node["async"].as<bool>();
        res.thread_pool_size = node["thread_pool_size"].as<size_t>();
        return res;
    }
};

/// @brief toString(GlobalConfig)
template <>
class LexicalCast<GlobalConfig, std::string> {
public:
    std::string operator()(const GlobalConfig& v) {
        YAML::Node node;
        node["async"] = v.async;
        node["thread_pool_size"] = v.thread_pool_size;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/*
 * ------------------------------------------------------------------------------
 * ------------------------------------------------------------------------------
 * ------------------------------------------------------------------------------
 */

void LogConfigInitializer::init() {
    //临时
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "/home/yjc/yjcServer/logs/system.log", true);
    // 创建一个名为 "system_logger" 的 logger
    auto logger =
        std::make_shared<spdlog::logger>("system_logger", file_sink);
    // 注册 logger，以便可以通过 spdlog::get() 访问它
    spdlog::register_logger(logger);

    auto logger_configs = Config::Lookup<std::vector<LoggerConfig>>(
        "loggers", {}, "logger_configs");
    auto global_configs =
        Config::Lookup<GlobalConfig>("global", {}, "global_configs");
    // TODO:绝对路径
    Config::LoadFromConfigDir("/home/yjc/yjcServer/template/ymls/");
}

LogConfigInitializer& LogConfigInitializer::instance() {
    static LogConfigInitializer m_LogConfig;
    return m_LogConfig;
}

/// @brief 初始化函数，由于构造函数，只会执行一次
LogConfigInitializer::LogConfigInitializer() {
    init();
}

}  // namespace yjcServer
