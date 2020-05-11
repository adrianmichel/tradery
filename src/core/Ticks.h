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

#include "cache.h"
#include "seriesimpl.h"

class TicksImpl : public Ticks, public Ideable {
  std::shared_ptr<SeriesImpl> _price;
  std::shared_ptr<SeriesImpl> _size;
  std::shared_ptr<TickTypeSeries> _type;
  std::shared_ptr<ExchangeSeries> _exchange;
  TimeSeries _time;

 public:
  TicksImpl(const std::string& dataSourceName, const std::string& symbol,
            const Range* range)
      : Ticks(symbol),
        Ideable(dataSourceName + " - ticks - " + symbol + (range == 0 ? "" : " - range: " + range->getId())),
        _price(std::make_shared< SeriesImpl >(getId() + " - tick price - ")),
        _size(std::make_shared< SeriesImpl >(getId() + " - tick size - ")),
        _type(std::make_shared < TickTypeSeries >()),
        _exchange(std::make_shared< ExchangeSeries >()) {}

  virtual ~TicksImpl() {}

  void add(const Tick& tick) {
    _price->push_back(tick.price());
    _size->push_back(tick.size());
    _type->push_back(tick.type());
    _exchange->push_back(tick.exchange());
    _time.push_back(tick.time());
  }

  size_t size() const {
    assert(_price->size() == _size->size());
    assert(_price->size() == _type->size());
    assert(_price->size() == _exchange->size());
    assert(_price->size() == _time.size());

    return _price->size();
  }

  virtual void forEach(TickHandler& tickHandler, size_t startBar = 0) const {
    if (startBar >= size())
      throw TickIndexOutOfRangeException(size(), startBar, getSymbol());

    for (size_t bar = startBar; bar < size(); bar++) {
      tickHandler.dataHandler(*this, bar);
    }
  }

  virtual const Tick get(size_t index) const {
    return Tick(_time.at(index), _price->at(index), (unsigned long)_size->at(index), _type->at(index), _exchange->at(index));
  }

  bool hasInvalidData() const { return false; }

  std::string getInvalidDataAsString() const { return ""; }
};
