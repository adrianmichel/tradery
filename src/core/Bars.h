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

#include <algorithm>
#include "cache.h"
#include "seriesimpl.h"
#include <objcounter.h>

using tradery::Bar;

class InvalidBars {
 private:
  // indexes of bars with invalid data
  std::vector<std::string> _invalidBarsMessages;

 public:
  void add(const std::string& message) {
    _invalidBarsMessages.push_back(message);
  }

  operator bool() const { return !_invalidBarsMessages.empty(); }

  std::string toString(unsigned int n = 3) const {
    std::string str;

    std::vector<std::string>::size_type i = 0;
    for (; i < std::min< std::vector< std::string >::size_type>(n, _invalidBarsMessages.size()); i++) {
      if (i > 0) {
        str += "; ";
      }
      str += _invalidBarsMessages[i];
    }

    if (i < _invalidBarsMessages.size()) {
      str += "; ...";
    }
    return str;
  }
};

// TODO: add iterators and other stuff so I can use algorithms on this

class BarsImpl : public tradery::BarsAbstr, public BarsBase, public Ideable {
  OBJ_COUNTER(BarsImpl)
 private:
  // interval between bars in seconds. Usually it goes from 1 minute to 1 month
  const unsigned int _resolution;
  const Type _type;

  Series _lowSeries;
  Series _highSeries;
  Series _openSeries;
  Series _closeSeries;
  Series _volumeSeries;
  Series _openInterest;
  tradery::TimeSeries _timeSeries;
  tradery::ExtraInfoSeries _extraInfoSeries;

  Synchronizer::SynchronizerPtr _synchronizer;

  ErrorHandlingMode _errorHandlingMode;

  InvalidBars _invalidBars;

 public:
  // TODO: the bars id is the symbol for now (for testing). Needs to have
  // datasource, range, and other info that shows it's up to date
  // and the various series are created attaching the name of the series
  // will need to be changed to make them foolproof, unique numbers etc
  BarsImpl(const std::string& dataSourceName, const std::string& symbol, Type type, unsigned int resolution, DateTimeRangePtr range, ErrorHandlingMode errorHandlingMode)
      : _resolution(resolution),
        _type(type), Ideable(dataSourceName + " - bars - " + symbol + (range == 0 ? "" : " - range: " + range->getId())),
        BarsBase(symbol), _errorHandlingMode(errorHandlingMode) {}

  ~BarsImpl() override {}

 public:
  void synchronize(Bars bars) override {
    _synchronizer = Synchronizer::create(bars, Bars(this));
    _lowSeries.synchronize(_synchronizer);
    _highSeries.synchronize(_synchronizer);
    _openSeries.synchronize(_synchronizer);
    _closeSeries.synchronize(_synchronizer);
    _volumeSeries.synchronize(_synchronizer);
    _openInterest.synchronize(_synchronizer);
    _timeSeries.synchronize(_synchronizer);
  }

  ErrorHandlingMode getErrorHandlingMode() const override {
    return _errorHandlingMode;
  }

  const InvalidBars& getInvalidBars() const { return _invalidBars; }

  bool hasInvalidData() const override { return (bool)_invalidBars; }

  std::string getInvalidDataAsString() const override {
    return _invalidBars.toString();
  }

  bool isSynchronized() const { return (bool)_synchronizer; }
  bool syncModified() const override {
    return isSynchronized() ? _synchronizer->modified() : false;
  }

  Type type() const override { return _type; }

  Synchronizer::SynchronizerPtr synchronizer() const { return _synchronizer; }
  size_t unsyncSize() const override { return _lowSeries.unsyncSize(); }
  std::pair<DateTime, DateTime> unsyncStartEnd() const override {
    return _timeSeries.unsyncStartEnd();
  }

  const Bar getBar(size_t ix) const override {
    return get(ix);
  }

  const std::string& getSymbol() const override { return BarsBase::getSymbol(); }

  const Bar get(size_t index) const override {
    return Bar( time(index), open(index), high(index), low(index), close(index),
        volume(index), openInterest(index), getBarExtraInfo(index) == 0 ? 0 : getBarExtraInfo(index)->clone());
  }

  unsigned long resolution() const override { return _resolution; }

  size_t size() const {
    if (_lowSeries.size() != _highSeries.size()) {
      LOG(log_info, "symbol: ", getSymbol(), "low size: ", _lowSeries.size(), ", high size: ", _highSeries.size());
    }
    assert(_lowSeries.size() == _highSeries.size());
    assert(_lowSeries.size() == _openSeries.size());
    assert(_lowSeries.size() == _closeSeries.size());
    assert(_lowSeries.size() == _volumeSeries.size());
    // todo: handle the time series for synchronized series
    assert(_lowSeries.unsyncSize() == _timeSeries.size());
    assert(_lowSeries.size() == _openInterest.size());
    // todo: handle extra info for synced series
    assert(_lowSeries.unsyncSize() == _extraInfoSeries.size());

    // this works for both sync and unsync bars - if it's synced, we already
    // synced the low series too (see synchronize( .. ) )
    return _lowSeries.size();
  }

  // implemented from base class Addable
  void add(const Bar& bar) {
    if (!bar.isValid()) {
      if (_errorHandlingMode == fatal) {
        throw BarException(bar.getStatusAsString());
      }
      else if (_errorHandlingMode == warning) {
        // if warning mode, add the index of the current bar (the one that's
        // being added)
        _invalidBars.add(bar.getStatusAsString());
      }
    }
    _lowSeries.push_back(bar.getLow());
    _highSeries.push_back(bar.getHigh());
    _openSeries.push_back(bar.getOpen());
    _closeSeries.push_back(bar.getClose());
    _volumeSeries.push_back(bar.getVolume());
    _openInterest.push_back(bar.getOpenInterest());
    _timeSeries.push_back(bar.time());
    _extraInfoSeries.push_back(bar.getBarExtraInfo());
  }

  void forEach(tradery::BarHandler& barHandler, size_t startBar = 0) const override {
    if (startBar >= size()) {
      throw BarIndexOutOfRangeException(size(), startBar, getSymbol());
    }

    for (size_t bar = startBar; bar < size(); bar++) {
      barHandler.dataHandler(*this, bar);
    }
  }

  double open(size_t barIndex) const {
    try {
      return _openSeries[barIndex];
    }
    catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
    catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
  }
  double low(size_t barIndex) const {
    try {
      return _lowSeries[barIndex];
    }
    catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
    catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
  }
  double high(size_t barIndex) const {
    try {
      return _highSeries[barIndex];
    }
    catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
    catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
  }
  double close(size_t barIndex) const {
    try {
      return _closeSeries[barIndex];
    }
    catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
    catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
  }
  unsigned long volume(size_t barIndex) const {
    try {
      return (unsigned long)_volumeSeries[barIndex];
    }
    catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
    catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
  }

  unsigned long openInterest(size_t barIndex) const {
    try {
      return (unsigned long)_openInterest[barIndex];
    }
    catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
    catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
  }

  Date date(size_t barIndex) const {
    return time(barIndex).date();
  }

  DateTime time(size_t barIndex) const {
    try {
      return timeSeries()[barIndex];
    }
    catch (const std::out_of_range&) {
      throw BarIndexOutOfRangeException(size(), barIndex, BarsBase::getSymbol());
    }
    catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
  }

  const tradery::BarExtraInfoPtr getBarExtraInfo(size_t barIndex) const {
    try {
      // todo: fix for synced series
      return 0;
      //      return _extraInfoSeries[ barIndex ] );
    }
    catch (const std::out_of_range&) {
      throw BarIndexOutOfRangeException(size(), barIndex, BarsBase::getSymbol());
    }
    catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(), BarsBase::getSymbol());
    }
  }

  const Series openSeries() const { return _openSeries; }
  const Series lowSeries() const { return _lowSeries; }
  const Series highSeries() const { return _highSeries; }
  const Series closeSeries() const { return _closeSeries; }
  const Series volumeSeries() const { return _volumeSeries; }
  const Series openInterestSeries() const { return _volumeSeries; }
  tradery::TimeSeries timeSeries() const {
    return isSynchronized() ? _synchronizer->timeSeries() : _timeSeries;
  }

  const tradery::ExtraInfoSeries& getExtraInfoSeries() const override {
    return _extraInfoSeries;
  }

  const Series TrueRange() const override;
  const Series AccumDist() const override;
  const Series ADX(unsigned int period) const override;
  const Series MidPrice(unsigned int period) const override;
  const Series SAR(double acceleration, double maximum) const override;
  const Series MFI(unsigned int period) const override;
  const Series CCI(unsigned int period) const override;
  const Series DX(unsigned int period) const override;
  const Series PlusDI(unsigned int period) const override;
  const Series PlusDM(unsigned int period) const override;
  const Series WillR(unsigned int period) const override;
  const Series NATR(unsigned int period) const override;
  const Series ChaikinAD() const override;
  const Series ChaikinADOscillator(unsigned int fastPeriod, unsigned int slowPeriod) const override;
  const Series OBV(const Series& series) const override;
  const Series AvgPrice() const override;
  const Series MedPrice() const override;
  const Series TypPrice() const override;
  const Series WclPrice() const override;
  const Series MinusDI(unsigned int period) const override;
  const Series MinusDM(unsigned int period) const override;
  const Series ADXR(unsigned int period) const override;
  const Series ATR(unsigned int period) const override;
  const Series TR() const override;
  const Series StochSlowK(int fastKPeriod, int slowKPeriod, MAType slowKMAType, int slowDPeriod, MAType slowDMAType) const override;
  const Series StochSlowD(int fastKPeriod, int slowKPeriod, MAType slowKMAType, int slowDPeriod, MAType slowDMAType) const override;
  const Series StochFastK(int fastKPeriod, int fastDPeriod, MAType fastDMAType) const override;
  const Series StochFastD(int fastKPeriod, int fastDPeriod, MAType fastDMAType) const override;

  const Series BOP() const override;

  // pattern recognition
  const Series Cdl3BlackCrows() const override;
  const Series CdlAbandonedBaby(double penetration) const override;
  const Series CdlDojiStar() const override;
  const Series CdlEngulfing() const override;
  const Series CdlEveningDojiStar(double penetration) const override;
  const Series CdlEveningStar(double penetration) const override;
  const Series CdlHammer() const override;
  const Series CdlHangingMan() const override;
  const Series CdlHarami() const override;
  const Series CdlHaramiCross() const override;
  const Series CdlHighWave() const override;
  const Series CdlIdentical3Crows() const override;
  const Series CdlInvertedHammer() const override;
  const Series CdlLongLine() const override;
  const Series CdlMorningDojiStar(double penetration) const override;
  const Series CdlMorningStar(double penetration) const override;
  const Series CdlShootingStar() const override;
  const Series CdlShortLine() const override;
  const Series CdlSpinningTop() const override;
  const Series CdlTristar() const override;
  const Series CdlUpsideGap2Crows() const override;
  const Series CdlHikkake() const override;

  // ta-lib 0.1.3
  const Series Cdl2Crows() const override;
  const Series Cdl3Inside() const override;
  const Series Cdl3LineStrike() const override;
  const Series Cdl3WhiteSoldiers() const override;
  const Series Cdl3Outside() const override;
  const Series CdlInNeck() const override;
  const Series CdlOnNeck() const override;
  const Series CdlPiercing() const override;
  const Series CdlStalled() const override;
  const Series CdlThrusting() const override;
  const Series CdlAdvanceBlock(double penetration) const override;

  // ta-lib 0.1.4

  const Series Cdl3StarsInSouth() const override;
  const Series CdlBeltHold() const override;
  const Series CdlBreakaway() const override;
  const Series CdlClosingMarubuzu() const override;
  const Series CdlConcealingBabySwallow() const override;
  const Series CdlCounterattack() const override;
  const Series CdlDoji() const override;
  const Series CdlDragonFlyDoji() const override;
  const Series CdlGapSideBySideWhite() const override;
  const Series CdlGravestoneDoji() const override;
  const Series CdlHomingPigeon() const override;
  const Series CdlKicking() const override;
  const Series CdlKickingByLength() const override;
  const Series CdlLadderBottom() const override;
  const Series CdlLongLeggedDoji() const override;
  const Series CdlMarubozu() const override;
  const Series CdlMatchingLow() const override;
  const Series CdlRickshawMan() const override;
  const Series CdlRiseFall3Methods() const override;
  const Series CdlSeparatingLines() const override;
  const Series CdlStickSandwich() const override;
  const Series CdlTakuri() const override;
  const Series CdlTasukiGap() const override;
  const Series CdlUnique3River() const override;
  const Series CdlXSideGap3Methods() const override;
  const Series CdlMatHold(double penetration) const override;

  const std::string locationInfoToXML() const {
    return BarsBase::locationInfoToXML();
  }
};

class Stock : public BarsImpl {};
/*
class Future : public Bars
{
private:
  double _tick;
  std::auto_ptr< Series > _openInterestSeries;

public:
  Future( const std::wstring& dataSourceName, const std::wstring& symbol,
unsigned int resolution, double tick ) : Bars( dataSourceName, symbol,
resolution ), _tick( tick ), _openInterestSeries( Series::create() )
  {
  }
};
*/
