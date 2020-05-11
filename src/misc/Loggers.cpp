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

#include "StdAfx.h"
#include <fstream>
#include <boost\algorithm\string\replace.hpp>
#include <path.h>
#include <loggers.h>
#include <logfilemaintainer.h>
#include <misc.h>

using namespace std::string_literals;

namespace tradery {
	inline std::string time_stamp_file_name() {
		std::string ts = time_stamp();

		boost::algorithm::replace_all(ts, _T(":"), _T("_"));
		return ts;
	}


	MISC_API std::string FileLogger::makeFileName(const std::string& logFilePath, const std::string& logFileNameRoot, const std::string& logFileExt) {
		return tradery::ws2s( Path(logFilePath).makePath(logFileNameRoot + time_stamp_file_name() + logFileExt) );
	}

	MISC_API std::string FileLogger::makeFileName() {
		return makeFileName(m_config.path().c_str(), m_logFileNameRoot, m_config.ext());
	}

	MISC_API FileLogger::FileLogger(const LogFileConfig& config, const std::string& logFileNameRoot)
		: Logger(config), m_config(config), m_logFileNameRoot(logFileNameRoot) {
		if (!fs::exists(m_config.path())) {
			throw LogException("Log file path doesn't exist: \""s + config.path().c_str() + "\"");
		}
		else if (m_logFileNameRoot.empty()) {
			throw LogException("Log file name root is empty");
		}
		else if (config.ext().empty()) {
			throw LogException("Log file extension is empty");
		}
		changeFile();
	}

	MISC_API void FileLogger::log(Level level, const std::string& str) const {
		std::unique_lock< std::mutex> l(log_mx);
		if (m_ofs.is_open()) {
			m_ofs << str;
			m_ofs.flush();
		}
		else {
		}
	}

	MISC_API void FileLogger::maintain() {
		auto size = fs::file_size(m_logFileName.c_str());
		// max size is in Mb
		if (size > m_config.maxSize() * 1000000) {
			changeFile();
		}

		LogFileMaintainer maintainer(m_config.path(), m_config.ext(), m_config.compress(), m_config.maxCount());
		maintainer.maintain();
	}
}