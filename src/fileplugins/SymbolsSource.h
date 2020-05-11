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

using std::vector;

/**
 * Reads a list of symbols from a text file.
 * Symbols can be separeted by new line, space, tab, "," or ";".
 */

// TODO: make this thread safe in order to allow multiple systems to use the
// same iterator (when running the same system in multiple threads, and still
// want to run each symbol
// only once

class FileSymbol : public tradery::Symbol {
 private:
  const std::string& _path;

 public:
  FileSymbol(const std::string& symbol, const std::string& path)
      : Symbol(symbol), _path(path) {}

  const std::string& path() const { return _path; }
};

class FileSymbolsSource : public SymbolsSource {
 private:
  const std::string _fileName;

 public:
  FileSymbolsSource(const Info& info, const std::string& fileName)
      : _fileName(fileName), SymbolsSource(info) {
    try {
      StrListPtr s = getSymbols(fileName);
      for (auto v : *s ) {
        push_back(v);
      }
    }
    catch (const FileSymbolsParserException& e) {
      throw SymbolsSourceException(e.message());
    }
  }

  const std::string& fileName() const { return _fileName; }
  SymbolConstPtr makeSymbol(const_iterator& i) const {
    return std::make_shared< FileSymbol >(*i, fileName());
  }
};
