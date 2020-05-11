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

#include <mutex>
#include <optional>
#include <future>

namespace tradery {
	class LogFileMaintainer {
	private:
		std::mutex m_mx;
		std::optional< int  > m_maxCount;
		const std::string m_path;
		const std::string m_ext;
		const bool m_compress;

	public:
		LogFileMaintainer(const std::string& path, const std::string& ext, bool compress, std::optional< int > maxCount = std::nullopt);

		~LogFileMaintainer() {
		}

		void maintain();
	};
}
