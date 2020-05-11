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

/** @file
 *  \brief contains a string tokenizer class
 */

#include <functional>
#include <string>
#include <vector>
#include <algorithm>
#include <locale>
#include <optional>
#include <assert.h>

// For the case the default is a space.
// This is the default predicate for the Tokenize() function.
template <typename T>
class CIsSpace{
 public:
  bool operator()(T c) const {
    // isspace<char> returns true if c is a white-space character (0x09-0x0D or
    // 0x20)
    return ::isspace<T>(c) != 0;
  }
};

// For the case the separator is a comma
template <typename T>
class CIsComma  {
 public:
  bool operator()(T c) const { return (TCHAR(',') == c); }
};

// For the case the separator is a character from a set of characters given in a
// string
template <typename T>
class CIsFromString {
 private:
  std::basic_string<T> m_ostr;

 public:
  // Constructor specifying the separators
  CIsFromString(const std::basic_string<T>& rostr) : m_ostr(rostr) {}

  CIsFromString(const T* rostr) : m_ostr(rostr) {}

  bool operator()(T c) const {
    return m_ostr.find(c) != std::basic_string<T>::npos;
  }
};

// String Tokenizer
template <typename T, class Pred>
class CTokenizer : public std::vector<std::basic_string<T> > {
 public:
  // The predicate should evaluate to true when applied to a separator.
   CTokenizer(const std::basic_string<T>& str, const Pred& pred, std::optional< size_t > maxTokens = std::nullopt) {
     assert(!maxTokens || *maxTokens > 0);

    // First clear the results vector
    __super::clear();
    typename std::basic_string<T>::const_iterator it = str.begin();
    typename std::basic_string<T>::const_iterator itTokenEnd = str.begin();

    while (it != str.end()) {
      // Eat seperators
      while (it != str.end() && pred(*it)) {
        it++;
      }

      if (!maxTokens || __super::size() < (*maxTokens - 1)) {
        // Find next token
        itTokenEnd = find_if(it, str.end(), pred);
        // Append token to result
        if (it < itTokenEnd) {
          __super::push_back(std::basic_string<T>(it, itTokenEnd));
        }
        it = itTokenEnd;
      }
      else {
        __super::push_back(std::basic_string< T >(it, str.end()));
        return;
      }
    }
  }
};

using Sep = CIsFromString<char>;
using Tokenizer = CTokenizer<char, Sep>;
