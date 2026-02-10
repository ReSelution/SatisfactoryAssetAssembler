#pragma once

#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <utility> // std::forward

namespace SAP::Logger {

  // Standard-Loggername
  constexpr const char *defaultLoggerName = "SAP";

  class Logger {
  public:
    static Logger &getInstance();

    void initLogger(const std::string &name, spdlog::level::level_enum level = spdlog::level::info);

    std::shared_ptr<spdlog::logger> getLogger(const std::string &name);

  private:
    Logger() = default;
    ~Logger() = default;

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;

    std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> m_loggers;
    std::mutex m_mutex;
  };

  // ---------------------------------------------------------
  // Logger-Zugriff
  // ---------------------------------------------------------

  inline std::shared_ptr<spdlog::logger> getLogger(const std::string &name) {
    return Logger::getInstance().getLogger(name);
  }

  inline std::shared_ptr<spdlog::logger> getDefaultLogger() {
    return Logger::getInstance().getLogger(defaultLoggerName);
  }

  // ---------------------------------------------------------
  // Logging-Wrapper (type-safe)
  // ---------------------------------------------------------

  template<typename... Args> inline void logTrace(const char *name, fmt::format_string<Args...> fmt, Args &&...args) {
    getLogger(name)->trace(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> inline void logDebug(const char *name, fmt::format_string<Args...> fmt, Args &&...args) {
    getLogger(name)->debug(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> inline void logInfo(const char *name, fmt::format_string<Args...> fmt, Args &&...args) {
    getLogger(name)->info(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> inline void logWarn(const char *name, fmt::format_string<Args...> fmt, Args &&...args) {
    getLogger(name)->warn(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> inline void logError(const char *name, fmt::format_string<Args...> fmt, Args &&...args) {
    getLogger(name)->error(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  inline void logCritical(const char *name, fmt::format_string<Args...> fmt, Args &&...args) {
    getLogger(name)->critical(fmt, std::forward<Args>(args)...);
  }

} // namespace SAP::Logger
