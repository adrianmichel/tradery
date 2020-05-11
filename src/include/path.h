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

#include <filesystem>
#include <stdio.h>
#include <random>
#include <atlstr.h>
#include <optional>

namespace fs = std::filesystem;

class Path {
private:
	fs::path m_path;

	template< typename T > fs::path _makePath(T t) const {
		return t;
	}

	template< typename U, typename ... T > fs::path _makePath(U p, T ... t) const {
		fs::path px(p);
		return px /= _makePath(t...);
	}

public:
	Path() {}

	Path(const std::string& root_path)
		: m_path(root_path) {
	}
	Path(const char* root_path)
		: m_path(root_path) {
	}

	Path(const fs::path& p)
		: m_path(p){}

	const Path& operator=(const Path& p) {
		if (this != &p) {
			m_path = p.getPath();
		}
		return *this;
	}

	const Path& operator=(const fs::path& p) {
		m_path = p;
		return *this;
	}

	const fs::path& getPath() const {	return m_path;}
	fs::path& getPath() { return m_path;  }
	template< typename ... T > fs::path makePath(T...t) const {
		return _makePath(m_path, t...);
	}

	static bool isRelativePath(const std::string& path) {
		return Path(path).isRelativePath();
	}

	static bool isAbsolutePath(const std::string& path) {
		return !isRelativePath(path);
	}

	bool isRelativePath() const {
		return !m_path.has_root_path();
	}

	bool isAbsolutePath() const {
		return !isRelativePath();
	}

	bool exists() const {
		return fs::exists(m_path);
	}

	bool createDirectories() const {
		return fs::exists(m_path) || fs::create_directories(m_path);
	}

	operator fs::path() const {
		return m_path;
	}

	static Path make_tmp_path(const std::string& dir) {
		Path p(std::filesystem::temp_directory_path());
		return p.makePath(dir);
	}

	static std::optional< Path > create_tmp_dir(const std::string& dir) {
		Path p = make_tmp_path(dir);
		return p.createDirectories() ? std::optional< Path >(p) : std::nullopt;
	}

};
