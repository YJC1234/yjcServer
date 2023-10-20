#include <Config/LogConfig.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace yjcServer {

namespace fs = std::filesystem;

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

    /// @brief 从sinkConfig中解析sink
    std::shared_ptr<spdlog::sinks::sink> createSink() const {
        std::shared_ptr<spdlog::sinks::sink> sink;
        //解析类型
        if (type == "rotating_file_sink_mt") {
            sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                filename, max_size, max_files);
        } else if (type == "stdout_color_sink_mt") {
            sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        } else if (type == "stderr_color_sink_mt") {
            sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        } else if (type == "daily_file_sink_mt") {
            sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
                filename, 0, 0);
        } else if (type == "null_sink_st") {
            sink = std::make_shared<spdlog::sinks::null_sink_st>();
        } else {
            spdlog::get("system_logger")
                ->info("Unsupported sink type: " + type);
        }

        sink->set_pattern(pattern);
        return sink;
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

    /// @brief 从string中解析出level
    static spdlog::level::level_enum
    StringToLevel(const std::string& level_str) {
        static const std::unordered_map<std::string,
                                        spdlog::level::level_enum>
            level_map = {{"trace", spdlog::level::trace},
                         {"debug", spdlog::level::debug},
                         {"info", spdlog::level::info},
                         {"warn", spdlog::level::warn},
                         {"error", spdlog::level::err},
                         {"critical", spdlog::level::critical},
                         {"off", spdlog::level::off}};

        auto it = level_map.find(level_str);
        if (it != level_map.end()) {
            return it->second;
        } else {
            throw std::invalid_argument("Invalid log level string: " +
                                        level_str);
        }
    }

    //从LoggerConfig中解析出Logger
    std::shared_ptr<spdlog::logger> createLogger() const {
        auto logger = std::make_shared<spdlog::logger>(name);
        logger->set_level(StringToLevel(level));
        std::vector<spdlog::sink_ptr> _sinks;
        for (auto& _sinkConfig : sinks) {
            auto sink = _sinkConfig.createSink();
            _sinks.push_back(sink);
        }
        logger->sinks() = _sinks;
        return logger;
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
        if (node["type"].IsDefined()) {
            res.type = node["type"].as<std::string>();
        }
        if (node["filename"].IsDefined()) {
            res.filename = node["filename"].as<std::string>();
        }
        if (node["max_files"].IsDefined()) {
            res.max_files = node["max_files"].as<size_t>();
        }
        if (node["max_size"].IsDefined()) {
            res.max_size = node["max_size"].as<size_t>();
        }
        if (node["pattern"].IsDefined()) {
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
        if (node["name"].IsDefined()) {
            res.name = node["name"].as<std::string>();
        }
        if (node["level"].IsDefined()) {
            res.level = node["level"].as<std::string>();
        }
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
        if (node["async"].IsDefined()) {
            res.async = node["async"].as<bool>();
        }
        if (node["thread_pool_size"].IsDefined()) {
            res.thread_pool_size = node["thread_pool_size"].as<size_t>();
        }
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

/*
 *---------------------------------全局对象--------------------------------------
 */
auto logger_configs = Config::Lookup<std::vector<LoggerConfig>>(
    "loggers", {}, "logger_configs");
auto global_configs =
    Config::Lookup<GlobalConfig>("global", {}, "global_configs");

/*
 *----------------------------------------------------------------------------
 */

void LogConfigInitializer::init() {
    fs::path logpath("/home/yjc/yjcServer/logs/system.log");
    if (fs::exists(logpath)) {  //删掉上一次运行的日志文件
        fs::remove(logpath);
    }
    //临时
    auto logger =
        spdlog::basic_logger_mt("system_logger", logpath.string());

    //使用回调实现日志变更时的加载
    logger_configs->addListener(
        [](const std::vector<LoggerConfig>& old_value,
           const std::vector<LoggerConfig>& new_value) {
            spdlog::get("system_logger")->info("on_logger_conf_changed");
            //添加或修改新的
            for (auto& _loggerConfig : new_value) {
                auto it = std::find(old_value.begin(), old_value.end(),
                                    _loggerConfig);
                if (it == old_value.end()) {
                    //创建新的logger
                    auto newLogger = _loggerConfig.createLogger();
                    //删除临时的logger
                    if (spdlog::get(_loggerConfig.name) != nullptr) {
                        spdlog::drop(_loggerConfig.name);
                    }
                    spdlog::register_logger(newLogger);
                    spdlog::get("system_logger")
                        ->info("\nNew logger create!\nHere is its "
                               "information:{}\n",
                               LexicalCast<LoggerConfig, std::string>()(
                                   _loggerConfig));
                } else {
                    if (*it != _loggerConfig) {
                        //修改已经存在的logger
                        auto reLogger = _loggerConfig.createLogger();
                        spdlog::register_logger(reLogger);
                    }
                }
            }
            //删除旧的
            for (auto& _loggerConfig : old_value) {
                auto it = std::find(new_value.begin(), new_value.end(),
                                    _loggerConfig);
                if (it == new_value.end()) {  //旧的多余，删除
                    spdlog::drop(_loggerConfig.name);
                }
            }
        });

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
