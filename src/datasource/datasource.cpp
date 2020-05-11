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
#include "datasource.h"

using namespace tradery;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

class SymbolsIteratorImpl : public tradery::SymbolsIterator {
  friend SymbolsSource;

 private:
  const SymbolsSource::const_iterator _end;
  mutable std::recursive_mutex _mutex;
  SymbolsSource::const_iterator _i;
  const SymbolsSource& _sls;

 public:
  SymbolsIteratorImpl(const SymbolsSource& sls)
      : _i(sls.begin()), _end(sls.end()), _sls(sls) {}

 public:
  /**
   * Gets the next symbol
   *
   * This method should be thread safe if it is inteded to be used from more
   * than one thread at a time.
   *
   * @return A pointer to a SymbolInfo object if next is available, or 0 if no
   * more symbols
   */
  virtual SymbolConstPtr getNext() {
    std::scoped_lock lock(_mutex);
    return _i == _end ? SymbolConstPtr() : _sls.makeSymbol(_i++);
  }

  virtual void reset() {
    std::scoped_lock lock(_mutex);
    _i = _sls.begin();
  }

  virtual SymbolConstPtr getFirst() {
    std::scoped_lock lock(_mutex);
    reset();
    return getNext();
  }

  virtual SymbolConstPtr getCurrent() {
    std::scoped_lock lock(_mutex);
    return _i == _end ? SymbolConstPtr() : _sls.makeSymbol(_i);
  }

  virtual bool hasMore() {
    std::scoped_lock lock(_mutex);
    return _i != _end;
  }
};

tradery::SymbolsIteratorPtr tradery::SymbolsSource::makeIterator() {
  _iterators.push_back( std::make_shared< SymbolsIteratorImpl >( *this ) );
  return _iterators.back();
}
