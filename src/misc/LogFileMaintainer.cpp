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
#include <misc.h>
#include "logfilemaintainer.h"
#include "LogFiles.h"


class InactiveLogFiles : public LogFiles {
public:
	InactiveLogFiles(const fs::path& p, std::initializer_list< std::string > extensions)
		: LogFiles(p, extensions) {
		// remove the most recent file from vector, this is likely
		// the current log file and we don't need to worry about it
		if (!empty()) {
			m_files.pop_back();
		}
	}

	// removes the oldest files until there are no more than "count" left
// unless some or all can't be removed due to errors (logged), in which
// case there will be more than "count" files left.
	void removeTo(int count) {
		// attempts to remove files starting with the oldest, which is at the beginning of
		// the vector

		int maxRemove = m_files.size() - count;
		auto end = std::remove_if(m_files.begin(), m_files.end(),
			[this, &maxRemove](const fs::path& p)->bool {
			if (maxRemove <= 0) {
				return false;
			}
			std::error_code ec;
			if (fs::remove(p, ec)) {
				--maxRemove;
				return true;
			}
			else {
				return false;
			}
		}
		);

		m_files.erase(end, m_files.end());
	}
};

namespace tradery {
	void LogFileMaintainer::maintain() {
		if (m_maxCount) {
			// will consider compressed and uncompressed log files
			InactiveLogFiles files(fs::path( tradery::s2ws( m_path )), { m_ext, m_ext + ".bz2" });

			files.removeTo(*m_maxCount);
		}

		if (m_compress) {
			/*
			InactiveLogFiles files(m_path, { m_ext });
			BZ2Utils bz;
			if (!bz.Activate()) {
				// handle activation error
			}

			// compress old file
			for (const fs::path& file : files.getFiles()) {
				CString errorText;
				CString bzFileName(file.filename().c_str());
				bz.CompressFile(CString(file.parent_path().c_str()), CString(file.filename().c_str()), bzFileName, errorText);
			}
			*/
		}
	}

	LogFileMaintainer::LogFileMaintainer(const std::string& path, const std::string& ext, bool compress, std::optional< int > maxCount)
		: m_path(path), m_ext(ext), m_compress(compress), m_maxCount(maxCount)
	{

	}
}