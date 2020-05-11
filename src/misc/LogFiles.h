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
#include <boost\algorithm\string\predicate.hpp>
#include <path.h>

class LogFiles {
protected:
	using Files = std::vector< fs::path >;
	Files m_files;

public:
	LogFiles(const fs::path& p, std::initializer_list< std::string > extensions) {
		auto pred = [](wchar_t a, wchar_t b)->bool {
			return boost::algorithm::iequals(std::string(1, a), std::string(1, b));
		};

		for (auto ext : extensions) {
			for (const fs::path& file : fs::directory_iterator(p)) {
				if (boost::algorithm::ends_with(file.c_str(), ext, pred)) {
					m_files.push_back(file);
				}
			}
		}

		// this sorts the vector in increasing time stamp order,
		// meaning the most recent file will be at the end
		std::sort(m_files.begin(), m_files.end(),
			[](const fs::path& f1, fs::path& f2) {
			return fs::last_write_time(f1) < fs::last_write_time(f2);
		}
		);
	}

	const std::vector< fs::path >& getFiles() const { return m_files; }

	bool empty() const { return m_files.empty(); }
	size_t count() const { return m_files.size(); }
};
