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

#include <string>
#include <mutex>
#include <sstream>
#include <iomanip>
#include "stringformat.h"

enum Level { log_debug, log_info, log_error, log_assert, log_any };

namespace tradery {

	class LogConfig {
	private:
		Level m_level;

	public:
		LogConfig(Level level)
			: m_level(level)
		{}

		Level level() const { return m_level; }
	};

	class LogFileConfig : public LogConfig {
	private:
		const std::string m_path;
		const std::string m_ext;
		const int m_maxCount;
		const long m_maxSize;
		const bool m_compress;
	public:
		LogFileConfig(const std::string& path, const std::string& ext,
			Level level, int maxCount, long maxSize, bool compress)
			: LogConfig(level), m_path(path), m_ext(ext), m_maxCount(maxCount),
			m_maxSize(maxSize), m_compress(compress)
		{}

		const std::string& path() const { return m_path; }
		const std::string& ext() const { return m_ext; }
		int maxCount() const { return m_maxCount; }
		long maxSize() const { return m_maxSize; }
		bool compress() const { return m_compress; }
	};

	class LogException : public std::exception {
	public:
		LogException(const std::string& message)
			: std::exception(message.c_str()) {}
	};


	inline std::string time_stamp() {
		time_t now = time(NULL);
		tm timeinfo;

		gmtime_s(&timeinfo, &now);
		return tradery::format(std::put_time(&timeinfo, "%FT%TZ"));
	}

	class Logger {
	protected:
		mutable std::mutex log_mx;
		const LogConfig m_config;
	public:
		virtual ~Logger() {}

		Logger(const LogConfig& config)
			: m_config( config){}
		virtual void log(Level level, const std::string& str) const = 0;
		virtual void maintain() = 0;
		char level() const {
			return m_config.level();
		}

	};
}
