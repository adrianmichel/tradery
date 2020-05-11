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
#include <algorithm>
#include <tchar.h>
#include <sstream>
#include <codecvt>
#include "traderytypes.h"

using namespace std::string_literals;

namespace tradery {

/**\defgroup String String related types
 * Definition of string related types
 * @{
 */

// transforms s in lower case
inline std::string to_upper_case(const std::string& str) {
  std::string s(str);

  std::transform(s.begin(), s.end(), s.begin(), toupper);
  return s;
}

inline std::string to_lower_case(const std::string& str) {
  std::string s(str);
  std::transform(s.begin(), s.end(), s.begin(), tolower);
  return s;
}

const std::string whiteSpaces(" \t\n\r");

// removes leading and trailing white space
inline std::string trim(const std::string& str) {
  std::wstring::size_type start = str.find_first_not_of(whiteSpaces);
  std::wstring::size_type end = str.find_last_not_of(whiteSpaces);

  if (start == std::wstring::npos)
    // empty string
    return "";
  else if (end == std::wstring::npos)
    return str.substr(start);
  else
    return str.substr(start, end - start + 1);
}

inline bool isBlanc(const std::string& str) {
  return str.find_first_not_of(whiteSpaces) == std::wstring::npos;
}

inline std::string addExtension(const std::string& fileName,
                                      const std::string& ext) {
  std::string result(fileName);
  if (!ext.empty()) result += std::string(ext[0] == '.' ? "" : ".") + ext;

  return result;
}

inline std::string addFSlash(const std::string& str) {
  if (str.empty() || *str.rbegin() == '\\' || *str.rbegin() == '/') {
    return str;
  }
  else {
    return str + '\\';
  }
}

inline std::string removeFSlash(const std::string& str) {
  if (str.empty() || *str.rbegin() != '\\') {
    return str;
  }
  else {
    return str.substr(0, str.length() - 1);
  }
}

// end string related classes
//@}
}  // namespace tradery

#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)
