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

///
/// @cond
///
namespace tradery {
///
/// @endcond
///

/**
 * Template class - represents a class (generally a collection) to which we can
 * add elements
 */
template <class T>
class Addable {
 public:
  virtual ~Addable() {}

  virtual void add(const T& t) = 0;
};

class FileSymbolsParserException {
 private:
  const std::string _message;

 public:
  FileSymbolsParserException(const std::string& message) : _message(message) {}

  const std::string& message() const { return _message; }
};

class SymbolTransformer {
 public:
  virtual ~SymbolTransformer() {}

  virtual std::string operator()(const std::string& symbol) const = 0;
};

using UniqueSymbolsSetBase = std::set<std::string>;

class UniqueSymbolsSet : public UniqueSymbolsSetBase {
 private:
  std::mutex _mx;

 public:
  bool add(const std::string& symbol) {
    return __super::insert(tradery::to_lower_case(symbol)).second;
  }

  bool hasSymbol(const std::string& symbol) const {
    return __super::find(tradery::to_lower_case(symbol)) != __super::end();
  }

  const UniqueSymbolsSet& operator+=(const UniqueSymbolsSet& symbols) {
    __super::insert(symbols.begin(), symbols.end());

    return *this;
  }
};

using UniqueSymbolsSetPtr = std::shared_ptr<UniqueSymbolsSet>;

StrListPtr getSymbols(const std::string& fileName, bool removeDuplcateSymbols = false, const SymbolTransformer* st = 0);
StrListPtr getSymbols(const StrVector& fileNames, bool removeDuplcateSymbols = false, const SymbolTransformer* st = 0, const tradery::StrList* extraSymbols = 0);

UniqueSymbolsSetPtr getUniqueSymbols(const StrVector& fileNames);
UniqueSymbolsSetPtr getUniqueSymbols(const std::string& file);
UniqueSymbolsSetPtr getUniqueSymbols(const std::string file1, const std::string& file2);

}  // namespace tradery
