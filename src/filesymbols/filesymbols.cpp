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

using namespace tradery;

class Symbols {
 private:
  StrListPtr _symbols;

  bool _uniqueSymbolsPopulated;
  UniqueSymbolsSetPtr _uniqueSymbols;
  const bool _removeDuplicateSymbols;
  const SymbolTransformer* _st;

  mutable std::mutex _mx;

 protected:
  Symbols(bool removeDuplicateSymbols, const SymbolTransformer* st = 0)
      : _removeDuplicateSymbols(removeDuplicateSymbols), _st(st),
        _symbols(std::make_shared< StrList >()), _uniqueSymbols(std::make_shared< UniqueSymbolsSet >()),
        _uniqueSymbolsPopulated(false) {}

  void parse(std::istream& is) {
    std::string str;

    do {
      std::getline(is, str);
      if (!str.empty()) {
        parseLine(str);
      }
    } while (!is.eof());
  }

  template <typename Container>
  void add(Container container) {
    for (auto v : container ) {
      // add the symbol only if the symbol didn't exist already in the set, and
      // if the _removeDuplicateSymbols flag is set
      if (!_removeDuplicateSymbols || _uniqueSymbols->add(v)) {
        _symbols->push_back((_st == 0 ? v : (*_st)(v)));
      }
    }
  }

  void parseLine(const std::string& str) {
    // TODO: allow for quoted strings in order to support spaces in names.
    if (str.at(0) == '#' || str.length() > 1 && str.at(0) == '/' && str.at(1) == '/') {
      return;
    }

    Tokenizer tokens(str, ",; \t");

    add(tokens);
  }

 public:
  StrListPtr get() { return _symbols; }
  UniqueSymbolsSetPtr getUniqueSymbolsSet() {
    if (!_removeDuplicateSymbols && !_uniqueSymbolsPopulated) {
      for (auto symbol : *_symbols) {
        _uniqueSymbols->add(symbol);
      }

      _uniqueSymbolsPopulated = true;
    }
    return _uniqueSymbols;
  }
};

/**
 * Loads a lost of symbols from a text file
 *
 * Symbols separators are: ",; \t" and new line
 */
class FileSymbolsList : public Symbols {
 private:
  bool _removeDuplicateSymbols;

 public:
  /**
   * Constructor that takes as arguments the file name and a flag indicating
   * whether to eliminate duplicate symbols
   *
   * @param fileName file name
   * @param removeDuplicateSymbols
   *                 Indicates whether to eliminate duplicate symbols
   *
   * @exception FileSymbolsParserException
   */
  FileSymbolsList(const std::string& fileName, bool removeDuplicateSymbols,const SymbolTransformer* st = 0)
      : Symbols(removeDuplicateSymbols, st) {
    (*this)(fileName);
  }

  /**
   * Constructor that takes as argument a vector of filenames, and flag
   * indicating whether to remove duplicate symbols
   *
   * @param fileNames vector of file names
   * @param removeDuplicateSymbols
   *                  eliminate duplicate symbols if true
   *
   * @exception FileSymbolsParserException
   */
  FileSymbolsList(const StrVector& fileNames, bool removeDuplicateSymbols, const SymbolTransformer* st = 0, const tradery::StrList* extraSymbols = 0)
      : Symbols(removeDuplicateSymbols, st) {
    for (auto fileName : fileNames) {
      (*this)(fileName);
    }

    if (extraSymbols != 0) {
      __super::add(*extraSymbols);
    }
  }

  bool operator()(const std::string& fileName) {
    if (!fileName.empty()) {
      std::ifstream file(fileName.c_str());
      if (!file) {
        throw FileSymbolsParserException("Could not open symbols file \""s + fileName + "\"");
      }
      __super::parse(file);
      return true;
    }
    else {
      throw FileSymbolsParserException(std::string("Symbols file name is empty"));
    }
  }
};

void deleteSymbols(const StrList* list) { delete list; }

StrListPtr tradery::getSymbols(
    const std::string& fileName, bool removeDuplicateSymbols, const SymbolTransformer* st) {
  // the reason I'm using a deleter function is so the deletion happens in the
  // same module that created the list, or memory errors will occur.
  return FileSymbolsList(fileName, removeDuplicateSymbols, st).get();
}

StrListPtr tradery::getSymbols(const StrVector& fileNames, bool removeDuplicateSymbols, const SymbolTransformer* st, const tradery::StrList* extraSymbols) {
  // the reason I'm using a deleter function is so the deletion happens in the
  // same module that created the list, or memory errors will occur.
  return FileSymbolsList(fileNames, removeDuplicateSymbols, st, extraSymbols).get();
}

UniqueSymbolsSetPtr tradery::getUniqueSymbols(const StrVector& fileNames) {
  return FileSymbolsList(fileNames, true).getUniqueSymbolsSet();
}

UniqueSymbolsSetPtr tradery::getUniqueSymbols(const std::string& file) {
  StrVector files;

  files.push_back(file);

  return getUniqueSymbols(files);
}

UniqueSymbolsSetPtr tradery::getUniqueSymbols(const std::string file1, const std::string& file2) {
  StrVector files;

  files.push_back(file1);
  files.push_back(file2);

  return tradery::getUniqueSymbols(files);
}
