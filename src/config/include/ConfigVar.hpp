#pragma once

#include <cxxabi.h>

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "Logger.h"

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
            LOG_FATAL("ConfigVar::toString exeception: %s, because convert from %s(%s) to string",
                      e.what(),
                      getTypeName().c_str()
                      description_.c_str());
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
            LOG_FATAL("ConfigVar::FromString exeception: %s, because convert from string(%s) to %s",
                      e.what(),
                      val.c_str(),
                      getTypeName().c_str());
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