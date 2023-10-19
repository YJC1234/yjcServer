#pragma once

#include <Config/util.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/type_index.hpp>

namespace yjcServer {

/*
 * ---------------------------------------
 * ----------------配置模块----------------
 *----------------------------------------
 */

/// @brief 配置变量基类
class ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVarBase>;
    /// @brief 构造函数
    /// @param name 配置参数名称[0-9a-z_.]
    /// @param m_description 配置参数描述
    ConfigVarBase(const std::string& name,
                  const std::string& description = "")
        : m_name(name), m_description(description) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(),
                       ::tolower);
    }
    /// @brief 析构函数
    ~ConfigVarBase() {}

    /// @brief 返回配置参数名称
    const std::string& getName() const {
        return m_name;
    }

    /// @brief 返回配置参数
    const std::string& getDescription() const {
        return m_description;
    }

    /// @brief 转成字符串
    virtual std::string toString() = 0;

    /// @brief 从字符串初始化值
    virtual bool fromString(const std::string&) = 0;

    /// @brief 返回配置参数的类型名称
    virtual std::string getTypeName() = 0;

protected:
    std::string m_name;         //配置参数名称
    std::string m_description;  //配置参数描述
};

/*
 *------------------------------------------------------------
 *-------------------------类型转换----------------------------
 *------------------------------------------------------------
 */

/// @brief 类型转化模板类
/// @tparam F 源类型
/// @tparam T 目标类型
template <class F, class T>
class LexicalCast {
public:
    /// @brief 类型转换
    /// @param v 源类型
    /// @return v转换后的目标类型
    /// @exception 当类型不可转换时抛出异常
    T operator()(const F& v) {
        return boost::lexical_cast<T>(v);
    }
};

/// @brief 类型转换模板类偏特化，yaml String转化为std::vector<T>
/// @tparam T
template <class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator()(const std::string& v) {
        YAML::Node        node = YAML::Load(v);
        std::vector<T>    vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

/// @brief 类型转换模板类片特化(std::vector<T> 转换成 YAML String)
template <class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/// @brief 类型转换模板偏特化(yaml string转化为list<T>)
template <class T>
class LexicalCast<std::string, std::list<T>> {
public:
    std::list<T> operator()(const std::string& v) {
        YAML::Node        node = YAML::Load(v);
        std::list<T>      vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

/// @brief 模板类型偏特化（list<T>转化为yaml string)
template <class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::set<T>)
 */
template <class T>
class LexicalCast<std::string, std::set<T>> {
public:
    std::set<T> operator()(const std::string& v) {
        YAML::Node           node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream    ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::set<T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_set<T>)
 */
template <class T>
class LexicalCast<std::string, std::unordered_set<T>> {
public:
    std::unordered_set<T> operator()(const std::string& v) {
        YAML::Node                     node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream              ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::unordered_set<T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::map<std::string, T>)
 */
template <class T>
class LexicalCast<std::string, std::map<std::string, T>> {
public:
    std::map<std::string, T> operator()(const std::string& v) {
        YAML::Node                        node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream                 ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(
                std::make_pair(it->first.Scalar(),
                               LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::map<std::string, T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for (auto& i : v) {
            node[i.first] =
                YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成
 * std::unordered_map<std::string, T>)
 */
template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
    std::unordered_map<std::string, T> operator()(const std::string& v) {
        YAML::Node                                  node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream                           ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(
                it->first
                    .Scalar(),  //这里是因为第一个肯定是string，可以用scalar
                LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::unordered_map<std::string, T> 转换成
 * YAML String)
 */
template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for (auto& i : v) {
            node[i.first] =
                YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
/*
 *------------------------------------------------------------
 *------------------------------------------------------------
 *-----------------------------------------------------------
 */

/// @brief 配置参数模板子类,保存对应类型的参数值
/// @tparam T 参数的具体类型
/// @tparam FromStr 从std::string转换成T类型的仿函数
/// @tparam ToStr 从T转换成std::string的仿函数
///  std::string 为YAML格式的字符串
template <class T, class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVar<T>>;

    using on_change_cb =
        std::function<void(const T& old_value, const T& new_value)>;

    /// @brief 构造函数，通过 参数名-参数值-描述 构造ConfigVar
    /// @param name 参数名
    /// @param default_value 参数值
    /// @param description 描述
    ConfigVar(const std::string& name, const T& default_value,
              const std::string& description = "")
        : ConfigVarBase(name, description), m_val(default_value) {}

    /// @brief 参数值转换为YAML string,转换失败抛出异常
    std::string toString() override {
        try {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return ToStr()(m_val);
        }
        catch (std::exception& e) {
            spdlog::get("system_logger")
                ->error("Configure var::toString exception : {}, convert "
                        "{} to string name {}",
                        e.what(), typeid(T).name(), m_name);
        }
        return "";
    }

    /// @brief 从YAML string转化为参数的值，转换失败抛出异常
    bool fromString(const std::string& val) override {
        try {
            setValue(FromStr()(val));
            return true;
        }
        catch (std::exception& e) {
            spdlog::get("system_logger")
                ->error("Configure var::fromString exception : {}, convert "
                        "{} to string name {}",
                        e.what(), typeid(T).name(), m_name);
        }
        return false;
    }

    /// @brief 获取当前参数的值
    T getValue() {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_val;
    }

    /// @brief 设置当前参数的值，如果值发生变化，通知对应的回调函数
    void setValue(const T& v) {
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            if (v == m_val) {
                return;
            }
            for (auto& i : m_cbs) {
                i.second(m_val, v);
            }
        }
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_val = v;
    }

    /// @brief  返回参数类型名称
    std::string getTypeName() {
        return boost::typeindex::type_id<T>().pretty_name();
    }

    /// @brief 添加变化回调函数
    /// @return 返回该回调函数对应的唯一id，用于删除回调
    uint64_t addListener(on_change_cb cb) {
        static uint64_t                     s_fun_id = 0;
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        ++s_fun_id;
        m_cbs[s_fun_id] = std::move(cb);
        return s_fun_id;
    }

    /// @brief 删除变化回调函数
    /// @param key 被删除函数的唯一id
    void delListener(uint64_t key) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_cbs.erase(key);
    }

private:
    std::shared_mutex m_mutex;
    T                 m_val;
    //变更回调函数组, uint64_t key,要求唯一，一般可以用hash
    std::map<uint64_t, on_change_cb> m_cbs;
};

/// @brief ConfigVar的管理类，提供便捷的方法管理ConfigVar
class Config {
public:
    using ConfigVarMap =
        std::map<std::string, std::shared_ptr<ConfigVarBase>>;

    /// @brief
    /// 获取/创建对应参数名的配置参数，如果名称为name的参数存在，直接返回;如果不存在，直接创建然后用default_value赋值
    /// @param name 参数名称
    /// @param default_value 默认值
    /// @param description 参数描述
    /// @return
    /// 返回对应的配置参数，如果参数名存在但是类型不匹配，返回nullptr
    template <class T>
    static typename std::shared_ptr<ConfigVar<T>>
    Lookup(const std::string& name, const T& default_value,
           const std::string& description = "") {
        std::unique_lock<std::shared_mutex> lock(GetMutex());

        auto it = GetDatas().find(name);
        if (it != GetDatas().end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if (tmp) {
                spdlog::get("system_logger")
                    ->info("Lookup name = {} exists.", name);
                return tmp;
            } else {
                spdlog::get("system_logger")
                    ->error("Lookup name = {} exists but type not {} "
                            "realtype = {}, real is {}",
                            name, typeid(T).name(),
                            it->second->getTypeName(),
                            it->second->toString());
                return nullptr;
            }
        }

        if (name.find_first_not_of(
                "abcdefghikjlmnopqrstuvwxyz._0123456789") !=
            std::string::npos) {
            spdlog::error("Lookup name:{} is not valid.", name);
            throw std::invalid_argument(name);
        }

        typename std::shared_ptr<ConfigVar<T>> v(
            new ConfigVar<T>(name, default_value, description));
        GetDatas()[name] = v;
        return v;
    }

    /// @brief 查找name对应的配置参数，没有找到则返回nullptr
    /// @tparam T 类型
    /// @param name 名称
    template <class T>
    static typename std::shared_ptr<ConfigVar<T>>
    Lookup(const std::string& name) {
        std::shared_lock<std::shared_mutex> lock(GetMutex());

        auto it = GetDatas().find(name);
        if (it == GetDatas().end()) {
            return nullptr;
        }
        auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
        if (tmp) {
            spdlog::get("system_logger")
                ->info("Lookup name = {} exists.", name);
            return tmp;
        }
        spdlog::get("system_logger")
            ->error("Lookup name = {} exists but type not {} "
                    "realtype = {}, real is {}",
                    name, typeid(T).name(), it->second->getTypeName(),
                    it->second->toString());
        return nullptr;
    }

    /// @brief 从YAML::Node初始化配置模块
    static void LoadFromYaml(const YAML::Node& root);

    /// @brief 加载path文件夹中所有配置文件
    /// @param force
    static void LoadFromConfigDir(const std::string& path,
                                  bool               force = false);

    /// @brief 查找配置参数，返回配置参数基类
    /// @param name 配置参数名称
    static std::shared_ptr<ConfigVarBase>
    LookupBase(const std::string& name);

    /// @brief 遍历模块中所有配置项
    /// @param cb 配置项回调函数
    static void
    Visit(std::function<void(std::shared_ptr<ConfigVarBase>)> cb);

private:
    /// @brief 返回所有配置项(单例模式)
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }

    /// @brief 返回配置项的rwMutex
    static std::shared_mutex& GetMutex() {
        static std::shared_mutex s_mutex;
        return s_mutex;
    }
};

}  // namespace yjcServer
