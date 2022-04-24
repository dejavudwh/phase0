#pragma once

#include <cxxabi.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "LogMarco.h"

namespace phase0
{
template <typename T>
static const char* typeToName()
{
    static const char* name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return name;
}

class ConfigVarBase
{
public:
    using ptr = std::shared_ptr<ConfigVarBase>;

    ConfigVarBase(const std::string& name, const std::string& description)
        : name_(name), description_(description)
    {
        std::transform(name_.begin(), name_.end(), name_.begin(), tolower);
    }

    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return name_; }
    const std::string& getDescriptin() const { return description_; }

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;

    virtual std::string getTypeName() const = 0;

protected:
    std::string name_;
    std::string description_;
};

template <typename F, typename T>
class LexicalCast
{
public:
    T operator()(const F& v) { return boost::lexical_cast<T>(v); }
};

template <typename T>
class LexicalCast<std::string, std::vector<T>>
{
public:
    std::vector<T> operator()(const std::string& v)
    {
        YAML::Node nodes = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            ss.str("");
            ss << nodes[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }

        return vec;
    }
};

template <typename T>
class LexicalCast<std::vector<T>, std::string>
{
public:
    std::string operator()(const std::vector<T>& v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <class T>
class LexicalCast<std::string, std::list<T>>
{
public:
    std::list<T> operator()(const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i)
        {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::list<T>, std::string>
{
public:
    std::string operator()(const std::list<T>& v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <class T>
class LexicalCast<std::string, std::set<T>>
{
public:
    std::set<T> operator()(const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i)
        {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::set<T>, std::string>
{
public:
    std::string operator()(const std::set<T>& v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <class T>
class LexicalCast<std::string, std::unordered_set<T>>
{
public:
    std::unordered_set<T> operator()(const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i)
        {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::unordered_set<T>, std::string>
{
public:
    std::string operator()(const std::unordered_set<T>& v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <class T>
class LexicalCast<std::string, std::map<std::string, T>>
{
public:
    std::map<std::string, T> operator()(const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::map<std::string, T>, std::string>
{
public:
    std::string operator()(const std::map<std::string, T>& v)
    {
        YAML::Node node(YAML::NodeType::Map);
        for (auto& i : v)
        {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>>
{
public:
    std::unordered_map<std::string, T> operator()(const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string>
{
public:
    std::string operator()(const std::unordered_map<std::string, T>& v)
    {
        YAML::Node node(YAML::NodeType::Map);
        for (auto& i : v)
        {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <typename T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase
{
public:
    using ptr = std::shared_ptr<ConfigVar>;
    using OnChange = std::function<void(const T& oldValue, const T& newValue)>;

    ConfigVar(const std::string& name, const std::string& description, const T& defaultVal)
        : ConfigVarBase(name, description), val_(defaultVal)
    {
    }

    std::string toString() override
    {
        try
        {
            return ToStr()(val_);
        }
        catch (std::exception& e)
        {
            P0SYS_LOG_ERROR() << "ConfigVar::toString exeception: " << e.what()
                              << ", because can't convert from " << getTypeName() << "(" << description_
                              << ")"
                              << "to YAML";
        }

        return "";
    }

    bool fromString(const std::string& val) override
    {
        try
        {
            setValue(FromStr()(val));
        }
        catch (std::exception& e)
        {
            P0SYS_LOG_ERROR() << "ConfigVar::FromString exeception:" << e.what()
                              << ", because can't convert from YAML(" << val << ") to " + getTypeName();
        }

        return true;
    }

    const T getValue() { return val_; }

    void setValue(const T& val)
    {
        if (val == val_)
        {
            return;
        }
        for (auto& i : onChangeMap)
        {
            i.second(val_, val);
        }

        val_ = val;
    }

    std::string getTypeName() const override { return typeToName<T>(); }

    uint64_t addListener(OnChange cb)
    {
        static uint64_t funcId = 0;
        funcId++;
        onChangeMap[funcId] = cb;
        return funcId;
    }

    void deleteListener(uint64_t funcId) { onChangeMap.erase(funcId); }

    OnChange getListener(uint64_t funcId)
    {
        auto& val = onChangeMap.find();
        return val == onChangeMap.end() ? nullptr : val;
    }

    OnChange clearListener() { onChangeMap.clear(); }

private:
    T val_;
    std::unordered_map<uint64_t, OnChange> onChangeMap;
    static uint64_t funcId_;
};

}  // namespace phase0