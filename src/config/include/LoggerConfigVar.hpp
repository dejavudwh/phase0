// #include <memory>
// #include <string>
// #include "ConfigVar.hpp"
// #include "Logger.h"

// namespace phase0
// {
// template <>
// class LexicalCast<std::string, LogLevel>
// {
// public:
//     LogLevel operator()(const std::string& v)
//     {
//         YAML::Node node = YAML::Load(v);
//         std::stringstream ss;
//         ss << node;
//         if (getLogLevelStr(LogLevel::INFO) == "info")
//         {
//             return LogLevel::INFO;
//         }
//         return LogLevel::FATAL;
//     }
// };

// template <>
// class LexicalCast<LogLevel, std::string>
// {
// public:
//     std::string operator()(const LogLevel& level)
//     {
//         YAML::Node node(YAML::NodeType::Scalar);
//         node.push_back(getLogLevelStr(level));
//         std::stringstream ss;
//         ss << node;
//         return ss.str();
//     }
// };

// template <>
// class LexicalCast<std::string, std::shared_ptr<AsynFileAppender>>
// {
// public:
//     std::shared_ptr<AsynFileAppender> operator()(const std::string& v)
//     {
//         YAML::Node node = YAML::Load(v);
//         std::stringstream ss;
//         ss << node["buffer_size"];
//         int bufferSize = boost::lexical_cast<int>(ss.str());
//         ss.clear();
//         ss << node["log_file"];
//         auto logFile = LexicalCast<std::string, std::shared_ptr<LogFile>>()(ss.str());
//         return std::shared_ptr<AsynFileAppender>(new AsynFileAppender(bufferSize));
//     }
// };

// template <>
// class LexicalCast<std::shared_ptr<AsynFileAppender>, std::string>
// {
// public:
//     std::string operator()(const std::shared_ptr<AsynFileAppender> v)
//     {
//         YAML::Node node(YAML::NodeType::Map);
//         node["buffer_size"] = v->getBufferSize();
//         node["log_file"] = LexicalCast<std::shared_ptr<LogFile>, std::string>()(v->getLogFile());
//         std::stringstream ss;
//         ss << node;

//         return ss.str();
//     }
// };

// template <>
// class LexicalCast<std::string, Logger>
// {
// public:
//     Logger& operator()(const std::string& v)
//     {
//         YAML::Node node = YAML::Load(v);
//         std::stringstream ss;
//         ss << node["level"];
//         LogLevel level = LexicalCast<std::string, LogLevel>()(ss.str());
//         ss.clear();
//         ss << node["appender"];
//         Logger& logger = Logger::getInstance();
//         logger.setAppender(LexicalCast<std::string, std::shared_ptr<AsynFileAppender>>()(ss.str()));
//         return logger;
//     }
// };
// }  // namespace phase0