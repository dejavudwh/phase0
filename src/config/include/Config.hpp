#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "ConfigVar.hpp"
#include "LogMarco.h"

namespace phase0
{
class Config
{
public:
    using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::ptr>;

    template <typename T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name,
                                             const T& defaultVal,
                                             const std::string& description)
    {
        return lookup(name, defaultVal, description, true);
    }

    template <typename T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name)
    {
        ConfigVarMap& cvm = getDatas();
        auto it = cvm.find(name);
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void visit(std::function<void(ConfigVarBase::ptr)> cb)
    {
        ConfigVarMap& cvm = getDatas();
        for (auto it = cvm.begin(); it != cvm.end(); ++it)
        {
            cb(it->second);
        }
    }

    static void LoadFromConf(const std::string& path)
    {
        try
        {
            YAML::Node root = YAML::LoadFile(path);
            LoadFromYaml(root);
        }
        catch (...)
        {
            P0SYS_LOG_ERROR() << "Load config file: " << path << " failed";
        }
    }

private:
    template <typename T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name,
                                             const T& defaultVal,
                                             const std::string& description,
                                             bool add)
    {
        ConfigVarMap& configMap = getDatas();
        auto it = configMap.find(name);
        if (it != configMap.end())
        {
            typename ConfigVar<T>::ptr tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if (tmp != nullptr)
            {
                return tmp;
            }
            else
            {
                P0SYS_LOG_ERROR() << "Look up config failed: name=" << it->second->getTypeName()
                                  << "type conversion error, not " << TypeToName<T>() << ", real type is "
                                  << it->second->getTypeName();
                return nullptr;
            }
        }

        if (add)
        {
            verifyName(name);
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, description, defaultVal));
            configMap[name] = v;
            return v;
        }

        return nullptr;
    }

    static Config::ConfigVarMap& getDatas()
    {
        static Config::ConfigVarMap datas_;
        return datas_;
    }

    static ConfigVarBase::ptr LookupBase(const std::string& name)
    {
        ConfigVarMap& cvm = getDatas();
        const auto& it = cvm.find(name);
        return it == cvm.end() ? nullptr : it->second;
    }

    static void LoadFromYaml(const YAML::Node& root)
    {
        std::list<std::pair<std::string, const YAML::Node>> allNodes;
        listAllMember(root.Scalar(), root, allNodes);
        for (auto& node : allNodes)
        {
            std::string key = node.first;
            if (key.empty())
            {
                continue;
            }

            std::transform(key.begin(), key.end(), key.begin(), tolower);
            ConfigVarBase::ptr var = LookupBase(key);

            if (var != nullptr)
            {
                if (node.second.IsScalar())
                {
                    var->fromString(node.second.Scalar());
                }
                else
                {
                    std::stringstream ss;
                    ss << node.second;
                    var->fromString(ss.str());
                }
            }
        }
    }

    static void listAllMember(const std::string& prefix,
                              const YAML::Node& node,
                              std::list<std::pair<std::string, const YAML::Node>>& output)
    {
        verifyName(prefix);
        output.push_back(std::make_pair(prefix, node));
        if (node.IsMap())
        {
            for (auto it = node.begin(); it != node.end(); it++)
            {
                listAllMember(prefix.empty() ? it->first.Scalar() : prefix + '.' + it->first.Scalar(),
                              it->second,
                              output);
            }
        }
    }

    static void verifyName(const std::string& name)
    {
        const static std::string legalChar = "abcdefghikjlmnopqrstuvwxyz._012345678";
        if (name.find_first_not_of(legalChar) != std::string::npos)
        {
            P0SYS_LOG_ERROR() << "look up config name invalid: "<< name;
            throw std::invalid_argument(name);
        }
    }
};

}  // namespace phase0