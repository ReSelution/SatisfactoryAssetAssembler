#include "logger.hpp"

#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "spdlog/sinks/basic_file_sink.h"

namespace SAA::Logger {

  Logger &Logger::getInstance() {
    static Logger instance;
    return instance;
  }

  void Logger::initLogger(const std::string &name, spdlog::level::level_enum level /* = spdlog::level::info */) {
    // Falls Logger bereits existiert → einfach zurückkehren
    if (m_loggers.count(name) != 0U) {
      return;
    }

    try {
      // Konsolen-Sink
      auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      consoleSink->set_level(spdlog::level::info);
      consoleSink->set_pattern("[SAP] %n: %v%$");

      // File-Sink
      auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/sap.log", true);
      fileSink->set_level(spdlog::level::trace);
      fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

      std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};
      auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
      spdlog::register_logger(logger);

      logger->set_level(level);
      m_loggers[name] = std::move(logger);

    } catch (const spdlog::spdlog_ex &ex) {
      std::cerr << "Logger initialization failed for '" << name << "': " << ex.what() << '\n';
    }
  }

  std::shared_ptr<spdlog::logger> Logger::getLogger(const std::string &name) {
    std::scoped_lock lock(m_mutex);

    auto iter = m_loggers.find(name);
    if (iter != m_loggers.end()) {
      return iter->second;
    }

    // Falls Logger noch nicht existiert → automatisch erstellen
    initLogger(name);
    return m_loggers[name];
  }

} // namespace SAP::Logger
