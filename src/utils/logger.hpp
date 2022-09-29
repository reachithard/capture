#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include <spdlog/async.h>
#include <spdlog/fmt/bundled/printf.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>
#include <sstream>

#include "singleton.h"

class Logger final : public Singleton<Logger> {
 public:
  ~Logger() = default;

  Logger() = default;

  /// let Logger like stream
  struct LogStream : public std::ostringstream {
   public:
    LogStream(const spdlog::source_loc &_loc, spdlog::level::level_enum _lvl,
              std::string _prefix)
        : loc(_loc), lvl(_lvl), prefix(_prefix) {}

    ~LogStream() { flush(); }

    void flush() {
      Singleton<Logger>::Get().Log(loc, lvl, (prefix + str()).c_str());
    }

   private:
    spdlog::source_loc loc;
    spdlog::level::level_enum lvl = spdlog::level::info;
    std::string prefix;
  };

  bool Init(std::string log_path, bool console = true) {
    if (_is_inited) return true;
    try {
      // initialize spdlog
      constexpr std::size_t log_buffer_size = 32 * 1024;  // 32kb
      // constexpr std::size_t max_file_size = 50 * 1024 * 1024; // 50mb
      spdlog::init_thread_pool(log_buffer_size, 2);
      std::vector<spdlog::sink_ptr> sinks;
      auto daily_sink =
          std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_path, 0, 2);
      sinks.push_back(daily_sink);

      // auto file_sink =
      // std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path.string(),
      // true); sinks.push_back(file_sink);

#if defined(_DEBUG) && defined(WIN32)
      auto ms_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
      sinks.push_back(ms_sink);
#endif  //  _DEBUG

      if (console) {
        auto consoleSink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(spdlog::level::debug);
        consoleSink->set_pattern("%s(%#): [%l %D %T.%e %P %t %!] %v");
        sinks.push_back(consoleSink);
      }

      spdlog::set_default_logger(
          std::make_shared<spdlog::logger>("", sinks.begin(), sinks.end()));

      spdlog::set_pattern("%s(%#): [%l %D %T.%e %P %t %!] %v");
      spdlog::flush_on(spdlog::level::warn);
      spdlog::set_level(_log_level);
    } catch (std::exception_ptr e) {
      assert(false);
      return false;
    }
    _is_inited = true;
    return true;
  }

  void Shutdown() { spdlog::shutdown(); }

  template <typename... Args>
  void Log(const spdlog::source_loc &loc, spdlog::level::level_enum lvl,
           const char *fmt, const Args &...args) {
    spdlog::log(loc, lvl, fmt, args...);
  }

  template <typename... Args>
  void Printf(const spdlog::source_loc &loc, spdlog::level::level_enum lvl,
              const char *fmt, const Args &...args) {
    spdlog::log(loc, lvl, fmt::sprintf(fmt, args...).c_str());
  }

  spdlog::level::level_enum Level() { return _log_level; }

  void SetLevel(spdlog::level::level_enum lvl) {
    _log_level = lvl;
    spdlog::set_level(lvl);
  }

  void Flush(spdlog::level::level_enum lvl) { spdlog::flush_on(lvl); }

  static const char *GetShortName(std::string path) {
    if (path.empty()) return path.data();

    size_t pos = path.find_last_of("/\\");
    return path.data() + ((pos == path.npos) ? 0 : pos + 1);
  }

 private:
  std::atomic_bool _is_inited{false};
  spdlog::level::level_enum _log_level = spdlog::level::trace;
};

// got short filename(exlude file directory)
#define __FILENAME__ (Logger::GetShortName(__FILE__))

// use fmt lib, e.g. LOG_WARN("warn log, {1}, {1}, {2}", 1, 2);
#define LOG_TRACE(msg, ...)                                       \
  {                                                               \
    if (Singleton<Logger>::Get().Level() == spdlog::level::trace) \
      spdlog::log({__FILENAME__, __LINE__, __FUNCTION__},         \
                  spdlog::level::trace, msg, ##__VA_ARGS__);      \
  };
#define LOG_DEBUG(msg, ...)                                                 \
  spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::debug, \
              msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...)                                                 \
  spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::info, \
              msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...)                                                 \
  spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::warn, \
              msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)                                                    \
  spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::err, msg, \
              ##__VA_ARGS__)
#define LOG_FATAL(msg, ...)                                                    \
  spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::critical, \
              msg, ##__VA_ARGS__)

// use like sprintf, e.g. PRINT_WARN("warn log, %d-%d", 1, 2);
#define PRINT_TRACE(msg, ...)                                             \
  Singleton<Logger>::Get().Printf({__FILENAME__, __LINE__, __FUNCTION__}, \
                                  spdlog::level::trace, msg, ##__VA_ARGS__);
#define PRINT_DEBUG(msg, ...)                                             \
  Singleton<Logger>::Get().Printf({__FILENAME__, __LINE__, __FUNCTION__}, \
                                  spdlog::level::debug, msg, ##__VA_ARGS__);
#define PRINT_INFO(msg, ...)                                              \
  Singleton<Logger>::Get().Printf({__FILENAME__, __LINE__, __FUNCTION__}, \
                                  spdlog::level::info, msg, ##__VA_ARGS__);
#define PRINT_WARN(msg, ...)                                              \
  Singleton<Logger>::Get().Printf({__FILENAME__, __LINE__, __FUNCTION__}, \
                                  spdlog::level::warn, msg, ##__VA_ARGS__);
#define PRINT_ERROR(msg, ...)                                             \
  Singleton<Logger>::Get().Printf({__FILENAME__, __LINE__, __FUNCTION__}, \
                                  spdlog::level::err, msg, ##__VA_ARGS__);
#define PRINT_FATAL(msg, ...)                                             \
  Singleton<Logger>::Get().printf({__FILENAME__, __LINE__, __FUNCTION__}, \
                                  spdlog::level::critical, msg,           \
                                  ##__VA_ARGS__);

// use like stream , e.g. STM_WARN() << "warn log: " << 1;
#define STM_TRACE()                                         \
  Logger::LogStream({__FILENAME__, __LINE__, __FUNCTION__}, \
                    spdlog::level::trace, "")
#define STM_DEBUG()                                         \
  Logger::LogStream({__FILENAME__, __LINE__, __FUNCTION__}, \
                    spdlog::level::debug, "")
#define STM_INFO()                                          \
  Logger::LogStream({__FILENAME__, __LINE__, __FUNCTION__}, \
                    spdlog::level::info, "")
#define STM_WARN()                                          \
  Logger::LogStream({__FILENAME__, __LINE__, __FUNCTION__}, \
                    spdlog::level::warn, "")
#define STM_ERROR()                                         \
  Logger::LogStream({__FILENAME__, __LINE__, __FUNCTION__}, \
                    spdlog::level::err, "")
#define STM_FATAL()                                         \
  Logger::LogStream({__FILENAME__, __LINE__, __FUNCTION__}, \
                    spdlog::level::critical, "")

#endif  // _LOGGER_HPP_