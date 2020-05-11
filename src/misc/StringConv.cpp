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

#include "stdafx.h"
#include "misc.h"
#include <stringconv.h>
#include <atlstr.h>

namespace tradery {
	using shared_wstring = std::shared_ptr< std::wstring >;
	using shared_string = std::shared_ptr< std::string >;
	using shared_char_array = std::shared_ptr< char[] >;
	using shared_wchar_array = std::shared_ptr< wchar_t[] >;

	MISC_API std::wstring s2ws(const char* str) {
		if (!str) {
			return std::wstring{};
		}

		int nLen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, NULL);
		if (nLen > 0) {
			auto lpszW = shared_wchar_array(new WCHAR[nLen + sizeof(WCHAR)], std::default_delete<WCHAR[]>());
			MultiByteToWideChar(CP_UTF8, 0, str, -1, lpszW.get(), nLen);
			return std::wstring{ lpszW.get() };
		}
		else {
			return std::wstring{};
		}
	}

	MISC_API std::wstring s2ws(const std::string& str) {
		return s2ws(str.c_str());
	}


	MISC_API std::string ws2s(const wchar_t* wstr) {
		if (!wstr) {
			return std::string{};
		}

		int nLen = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, NULL, NULL, NULL);
		if (nLen > 0) {
			auto lpsz = shared_char_array(new char[nLen + sizeof(char)], std::default_delete<char[]>());
			WideCharToMultiByte(CP_UTF8, 0, wstr, -1, lpsz.get(), nLen, NULL, NULL);
			return std::string{ lpsz.get() };
		}
		else {
			return std::string{};
		}
	}

	MISC_API std::string ws2s(const std::wstring& wstr) {
		return ws2s(wstr.c_str());
	}
}