/*
   Copyright (C) 2018-2020 Adrian Michel

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include <sys/timeb.h>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "logger.h"

#include "misc.h"

#ifdef LOG_EXPORTS
#define LOG_API __declspec(dllexport)
#else
#define LOG_API __declspec(dllimport)
#endif

/**/

inline std::string hexstr(unsigned int n, unsigned int digits) {
  std::ostringstream os;
  os << "0x" << std::setw(digits) << std::setfill('0') << std::hex << n;
  return os.str();
}

#define HEXSTR(n, digits) hexstr(n, digits).c_str()
#define HEXSTR4(n) HEXSTR(n, 4)
#define HEXSTR8(n) HEXSTR(n, 8)
#define HEXSTR5(n) HEXSTR(n, 5)

namespace tradery {

class LOG_API Log {
 private:
  static std::string levelToString(Level level) {
    switch (level) {
      case log_debug:
        return "debug ";
      case log_info:
        return "info  ";
      case log_error:
        return "error ";
      case log_assert:
        return "assert";
      case log_any:
        return "any   ";
      default:
        assert(false);
        return "";
    }
  }

  static std::string header(Level level, const char* func) {
    assert(func != 0);
    return tradery::format( "[", timeStamp(true), " ", HEXSTR5(GetCurrentThreadId()), ",", 
      HEXSTR5(GetCurrentProcessId()), "][", levelToString(level), "] [", func, "] " );
  }

  static void setLevel(Level level);

private:
  std::vector< std::shared_ptr< Logger > > m_loggers;
  static Log m_log;

public:
  ~Log() {
  }

  Log& addLogger(std::shared_ptr< Logger > logger) {
    m_loggers.push_back(logger);
    return *this;
  }

  Log& enableMaintainer() {
    return *this;
  }

  void maintain() {
    for (auto logger : m_loggers) {
      logger->maintain();
    }
  }

  template< Level level, typename... T > std::string out(const char* function, T...t) const {
    std::ostringstream os;
    os << "[" << time_stamp() << "][" << std::setfill('0')
      << std::setw(6) << std::this_thread::get_id() << "][" << function << "] ";

    ((os << std::forward<T>(t)), ...);
    os << std::endl;

    return os.str();
  }

  template <Level level, typename... T> void log(const char* function, T... t) const {
    for (auto logger : m_loggers) {
      std::string str = out< level>(function, std::forward<T>(t)...);
      logger->log(level, str);
    }
  }

  static Log& log() {
    return m_log;
  }
};
}

#define LOG( level, ... ) \
  [&](const char* function, auto... args)->void{ \
  Log::log().log< level >( function, args...); \
}(__FUNCTION__, __VA_ARGS__)

class LogEntryExit {
 private:
  Level m_level;
  const std::string m_name;
  const std::string m_message;
  const unsigned __int64 m_id;

  LOG_API static unsigned __int64 m_crtId;

  LOG_API static std::mutex m_mx;

  static unsigned __int64 getId() {
    std::scoped_lock lock(m_mx);
    return m_crtId++;
  }

 public:
  LogEntryExit(Level level, char* name, const std::string& message)
      : m_level(level), m_name(name), m_message(message), m_id(getId()) {
    std::ostringstream os;
    os << m_message << " - entry [" << m_id << "]";
 //   tradery::Log::xlog(m_level, const_cast<char*>(m_name.c_str()), os.str());
  }

  ~LogEntryExit() {
    std::ostringstream os;
    os << m_message << " - exit [" << m_id << "]";
//    tradery::Log::xlog(m_level, const_cast<char*>(m_name.c_str()), os.str());
  }
};

#define LOG_ENTRY_EXIT(level, message) \
  LogEntryExit ___logentryexit___(level, __FUNCTION__, message);
