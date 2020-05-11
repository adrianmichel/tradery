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

namespace tradery {
  /**
   * A symbols list iterator
   *
   * Used to iterate through a list of symbols.
   *
   * Iterators are created only by instances of the class SymbolsListSource or
   * derived, never by the user code directly.
   *
   * @see SymbolsListSource
   */
  class DATASOURCE_API SymbolsIterator {
  public:
    virtual ~SymbolsIterator() {}
    /**
     * Gets the next symbol
     *
     * This method is thread safe - can be used from more than one thread at a
     * time.
     *
     * @return A pointer to a SymbolInfo object if next is available, or 0 if no
     * more symbols
     */
    virtual SymbolConstPtr getNext() = 0;
    virtual void reset() = 0;
    virtual SymbolConstPtr getFirst() = 0;
    virtual SymbolConstPtr getCurrent() = 0;
    virtual bool hasMore() = 0;
  };

  using SymbolsIteratorPtr = std::shared_ptr<SymbolsIterator>;

}