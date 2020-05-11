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

#include <fstream>
#include <iostream>
#include <mutex>
#include "misc.h"
#include "logger.h"

namespace tradery {

	class ConsoleLogger : public Logger {
	public:
		ConsoleLogger( const LogConfig& config)
			: Logger( config )
		{}
		virtual void log(Level level, const std::string& str) const {
			std::unique_lock< std::mutex> l(log_mx);
			std::cout << str;
		}

		virtual void maintain() {}
	};


	class MISC_API FileLogger : public Logger {
	private:
		const LogFileConfig m_config;
		const std::string m_logFileNameRoot;
		mutable std::string m_logFileName;
		mutable std::ofstream m_ofs;

	private:
		std::string makeFileName(const std::string& logFilePath, const std::string& logFileNameRoot, const std::string& logFileExt);
		std::string makeFileName();
		void changeFile() {
			std::unique_lock< std::mutex >(m_mx);
			m_logFileName = makeFileName();
			if (m_ofs.is_open()) {
				m_ofs.flush();
				m_ofs.close();
			}
			m_ofs.open( m_logFileName, std::ofstream::out | std::ofstream::app);
		}

	public:
		FileLogger(const LogFileConfig& config, const std::string& logFileNameRoot );
		virtual void log(Level level, const std::string& str) const;
		void maintain();
	};
}