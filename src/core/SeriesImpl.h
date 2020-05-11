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

using std::vector;

// TODO: after a lot of thinking, I chose to make this class a vector< double >
// instead of a templetized class, that takes the type of the
// Series element as argument, and this because I don't know how to make this
// usable polymorphically in other classes such as SMASeries, which cannot know
// at compile time all the series that it may need to work on. As every integral
// and double type converts to a double, this will work, at the expense of
// storing more info for things like int or short It could  have been done by
// making vector< T > a member of Series and adding all the needed vector
// members to the class Series to make it usable with algorithms, but the
// process would have been too tedious
class SeriesImpl : public tradery::SeriesAbstr, public Ideable {
 protected:
  mutable vector<double> _v;

 private:
  Synchronizer::SynchronizerPtr _synchronizer;

  int getIndex(size_t ix) const {
    return isSynchronized() ? _synchronizer->index(ix) : ix;
  }

 public:
  // copy constructor
  /*  SeriesImpl( const SeriesImpl& series, const Id& id = Id() )
    : vector< double >( series.size() ), Ideable(id )
    {
      std::copy( series.begin(), series.end(), begin() );
    }
  */
  SeriesImpl(size_t size, Synchronizer::SynchronizerPtr synchronizer, const Id& id = Id())
      : _v(size), Ideable(id), _synchronizer(synchronizer) {}

  SeriesImpl(const Id& id = Id()) : Ideable(id) {}

  SeriesImpl(const SeriesImpl& series)
      : _v(series.getVector()), Ideable(Id()), _synchronizer(series.synchronizer()) {}

  virtual ~SeriesImpl() {}

 public:
  void synchronize(Synchronizer::SynchronizerPtr synchronizer) override {
    _synchronizer = synchronizer;
  }

  bool isSynchronized() const override { return (bool)_synchronizer; }

  Synchronizer::SynchronizerPtr synchronizer() const override { return _synchronizer; }

  // not affected by synchronizer as it is used to calculate indicators, which
  // should be based on the unsynced data
  double& at(size_t index) { return _v.at(index); }

  void push_back(double value) {
    // todo: can't modify a synced series?
    _v.push_back(value);
  }

  double setValue(size_t barIndex, double value) override {
    try {
      // todo: should we throw an exception if the series is synced? Probably we
      // don't want to set values then
      if (isSynchronized()) {
        throw OperationNotAllowedOnSynchronizedseriesException();
      }
      _v.at(barIndex) = value;
      return value;
    }
    catch (const std::out_of_range&) {
      throw SeriesIndexOutOfRangeException(size(), barIndex);
    }
  }

  SeriesAbstrPtr clone() const override {
    return std::make_shared< SeriesImpl >(*this);
  }

  double& getRef(size_t ix) override {
    if (ix >= size()) {
      throw SeriesIndexOutOfRangeException(size(), ix);
    }
    return _v[ix];
  }

  double getValue(size_t ix) const override {
    try {
      return _v.at(getIndex(ix));
    }
    catch (const std::out_of_range&) {
      throw SeriesIndexOutOfRangeException(size(), ix);
    }
  }

  // this returns the synchronized size (the same as the actual size if not
  // synchronized)
  size_t size() const override {
    return isSynchronized() ? _synchronizer->size() : _v.size();
  }

  size_t unsyncSize() const override { return _v.size(); }

  const double* getArray() const override {
    return (const double*)(&_v.front());
  }

  const vector<double>& getVector() const override { return _v; }

 public:
  SeriesAbstrPtr operator*(SeriesAbstrPtr series) const override {
    return multiply(series);
  }
  SeriesAbstrPtr operator*(double value) const override {
    return multiply(value);
  }
  SeriesAbstr& operator*=(SeriesAbstrPtr series) override {
    _v = ((*this) * series)->getVector();
    return *this;
  }
  SeriesAbstr& operator*=(double value) override {
    _v = ((*this) * value)->getVector();
    return *this;
  }

  SeriesAbstrPtr operator+(SeriesAbstrPtr series) const override {
    return add(series);
  }
  SeriesAbstr& operator+=(SeriesAbstrPtr series) override {
    _v = ((*this) + series)->getVector();
    return *this;
  }

  SeriesAbstrPtr operator+(double value) const override { return add(value); }
  SeriesAbstr& operator+=(double value) override {
    _v = ((*this) + value)->getVector();
    return *this;
  }

  SeriesAbstrPtr operator-(SeriesAbstrPtr series) const {
    return subtract(series);
  }
  SeriesAbstr& operator-=(SeriesAbstrPtr series) {
    _v = ((*this) - series)->getVector();
    return *this;
  }

  SeriesAbstrPtr operator-(double value) const {
    return subtract(value);
  }
  SeriesAbstr& operator-=(double value) {
    _v = ((*this) - value)->getVector();
    return *this;
  }

  SeriesAbstrPtr operator/(SeriesAbstrPtr series) const override {
    return divide(series);
  }

  SeriesAbstr& operator/=(SeriesAbstrPtr series) override {
    _v = ((*this) / series)->getVector();
    return *this;
  }

  SeriesAbstrPtr operator/(double value) const override {
    return divide(value);
  }
  SeriesAbstr& operator/=(double value) override {
    _v = ((*this) / value)->getVector();
    return *this;
  }

  bool crossOver(size_t barIndex, SeriesAbstrPtr series) const  override {
    if (size() != series->size()) {
      throw OperationOnUnequalSizeSeriesException(size(), series->size());
    }
    // TODO: see how to handle this case - maybe an exception when barIndex ==
    // 0, as we cannot determine a crossover
    if (barIndex == 0) {
      throw InvalidIndexForOperationException(barIndex, "Series::crossOver");
    }

    return (getValue(barIndex - 1) < series->getValue(barIndex - 1)) &&
           (getValue(barIndex) >= series->getValue(barIndex));
  }

  bool crossOver(size_t barIndex, double d) const override {
    if (barIndex == 0) {
      throw InvalidIndexForOperationException(barIndex, "Series::crossOver");
    }

    return (getValue(barIndex - 1) < d) && (getValue(barIndex) >= d);
  }

  bool crossUnder(size_t barIndex, SeriesAbstrPtr series) const override {
    if (size() != series->size()) {
      throw OperationOnUnequalSizeSeriesException(size(), series->size());
    }
    // TODO: see how to handle this case - maybe an exception when barIndex ==
    // 0, as we cannot determine a crossover
    if (barIndex == 0) {
      throw InvalidIndexForOperationException(barIndex, "Series::crossUnder");
    }

    return (getValue(barIndex - 1) > series->getValue(barIndex - 1)) &&
           (getValue(barIndex) <= series->getValue(barIndex));
  }

  bool crossUnder(size_t barIndex, double d) const override {
    if (barIndex == 0) {
      throw InvalidIndexForOperationException(barIndex, "Series::crossUnder");
    }

    return (getValue(barIndex - 1) > d) && (getValue(barIndex) <= d);
  }

 public:
  // subtracts series from the current series and puts the result in dest (does
  // not create a new series)
  /*  void subtractSeries( const Series& series, Series& dest ) const
    {
      //TODO: should this be an exception, as the user may inadverdently use
    unequal series assert( size() == series.size() ); assert( size() ==
    dest.size() );

      std::transform( begin(), end(), series.begin(), dest.begin(), std::minus<
    double >() );
    }
  */

 public:
  bool turnDown(size_t barIndex) const override {
    // TODO: see what to do if barIndex is 0 or 1, as in this case it cannot say
    // whether it turned down or not
    // assert for now
    assert(barIndex > 1);

    return (getValue(barIndex - 1) >= getValue(barIndex - 2)) &&
           (getValue(barIndex) < getValue(barIndex - 1));
  }

  bool turnUp(size_t barIndex) const override {
    // TODO: see what to do if barIndex is 0 or 1, as in this case it cannot say
    // whether it turned down or not
    // assert for now
    assert(barIndex > 1);

    return (getValue(barIndex - 1) <= getValue(barIndex - 2)) &&
           (getValue(barIndex) > getValue(barIndex - 1));
  }

  // returns the index of the maximum value in the range starting at start and
  // of length "length" the index is relative to the start of the series
  size_t maxIndexInInterval(size_t start, size_t length) const {
    // TODO: validate the inputs
    assert(start < size());
    assert(length > 0);

    double _max = _v.at(start);
    size_t maxIndex = start;

    // TODO: use std algorithm
    for (size_t n = start + 1; n < min2(start + length, size()); n++) {
      if (_v.at(n) > _max) {
        _max = _v.at(n);
        maxIndex = n;
      }
    }
    return maxIndex;
  }

  // returns the index of the minimum value in the range
  // the index is relative to the start of the series
  size_t minIndexInInterval(size_t start, size_t length) const {
    // TODO: validate the inputs
    assert(start < size());
    assert(length > 0);

    double _min = _v.at(start);
    size_t minIndex = start;

    // TODO: use std algorithm
    for (size_t n = start + 1; n < start + length; n++) {
      if (_v.at(n) < _min) {
        _min = _v.at(n);
        minIndex = n;
      }
    }
    return minIndex;
  }

  // TODO:
  virtual SeriesAbstr& operator=(SeriesAbstrPtr series) override {
    if (size() != series->size()) {
      throw OperationOnUnequalSizeSeriesException(size(), series->size());
    }

    // todo - can the source and dest be the same?
    _v = series->getVector();
    return *this;
  }

  SeriesAbstrPtr shiftRight(size_t n) const override;
  SeriesAbstrPtr shiftLeft(size_t n) const override;

  SeriesAbstrPtr multiply(SeriesAbstrPtr series) const override;
  SeriesAbstrPtr multiply(double value) const override;
  SeriesAbstrPtr add(SeriesAbstrPtr series) const override;
  SeriesAbstrPtr add(double value) const override;
  SeriesAbstrPtr subtract(SeriesAbstrPtr series) const override;
  SeriesAbstrPtr subtract(double value) const override;
  SeriesAbstrPtr subtractFrom(double value) const override;
  SeriesAbstrPtr divide(SeriesAbstrPtr series) const override;
  SeriesAbstrPtr divide(double value) const override;
  // this is value/series
  SeriesAbstrPtr divideBy(double value) const override;

  // indicators
  SeriesAbstrPtr SMA(unsigned int period) const override;
  SeriesAbstrPtr EMA(unsigned int period) const override;
  SeriesAbstrPtr EMA(unsigned int period, double exp) const override;
  SeriesAbstrPtr WMA(unsigned int period) const override;
  SeriesAbstrPtr AroonDown(unsigned int period) const override;
  SeriesAbstrPtr AroonUp(unsigned int period) const override;
  //  virtual SeriesConstPtr AroonOscillator( const Series& series, unsigned int
  //  period ) const override;
  SeriesAbstrPtr ROC(unsigned int period) const override;
  //  virtual SeriesConstPtr MACDFix( const Series& series ) const override;
  SeriesAbstrPtr BBandUpper(unsigned int period, double stdDev) const override;
  SeriesAbstrPtr BBandLower(unsigned int period, double stdDev) const override;
  SeriesAbstrPtr DEMA(unsigned int period) const override;
  SeriesAbstrPtr HTTrendline() const override;
  SeriesAbstrPtr KAMA(unsigned int period) const override;
  SeriesAbstrPtr MAMA(double fastLimit, double slowLimit) const override;
  SeriesAbstrPtr FAMA(double fastLimit, double slowLimit) const override;
  SeriesAbstrPtr Min(unsigned int period) const override;
  SeriesAbstrPtr Max(unsigned int period) const override;
  SeriesAbstrPtr MinIndex(unsigned int period) const override;
  SeriesAbstrPtr MaxIndex(unsigned int period) const override;
  SeriesAbstrPtr MidPoint(unsigned int period) const override;
  SeriesAbstrPtr PPO(unsigned int fastPeriod, unsigned int slowPeriod, MAType maType) const override;
  SeriesAbstrPtr ROCP(unsigned int period) const override;
  SeriesAbstrPtr ROCR(unsigned int period) const override;
  SeriesAbstrPtr ROCR100(unsigned int period) const override;
  SeriesAbstrPtr RSI(unsigned int period) const override;
  SeriesAbstrPtr TRIX(unsigned int period) const override;
  SeriesAbstrPtr HTDCPeriod() const override;
  SeriesAbstrPtr HTDCPhase() const override;
  SeriesAbstrPtr HTPhasorPhase() const override;
  SeriesAbstrPtr HTPhasorQuadrature() const override;
  SeriesAbstrPtr HTSine() const override;
  SeriesAbstrPtr HTLeadSine() const override;
  SeriesAbstrPtr HTTrendMode() const override;
  SeriesAbstrPtr LinearReg(unsigned int period) const override;
  SeriesAbstrPtr LinearRegSlope(unsigned int period) const override;
  SeriesAbstrPtr LinearRegAngle(unsigned int period) const override;
  SeriesAbstrPtr LinearRegIntercept(unsigned int period) const override;
  SeriesAbstrPtr StdDev(unsigned int period, double nbDev) const override;
  SeriesAbstrPtr Variance(unsigned int period, double nbDev) const override;
  SeriesAbstrPtr Correlation(SeriesAbstrPtr series, unsigned int period) const override;
  SeriesAbstrPtr Beta(SeriesAbstrPtr series, unsigned int period) const override;
  SeriesAbstrPtr TSF(unsigned int period) const override;
  SeriesAbstrPtr CMO(unsigned int period) const override;
  SeriesAbstrPtr MOM(unsigned int period) const override;
  SeriesAbstrPtr Momentum(unsigned int period) const override;
  SeriesAbstrPtr MACD(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const override;
  SeriesAbstrPtr MACDSignal(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const override;
  SeriesAbstrPtr MACDHist(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const override;
  SeriesAbstrPtr MACDExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const override;
  SeriesAbstrPtr MACDSignalExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const override;
  SeriesAbstrPtr MACDHistExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const override;
  SeriesAbstrPtr MACDFix(unsigned int period) const override;
  SeriesAbstrPtr MACDSignalFix(unsigned int period) const override;
  SeriesAbstrPtr MACDHistFix(unsigned int period) const override;
  SeriesAbstrPtr APO(unsigned int fastPeriod, unsigned int slowPeriod, MAType maType) const override;
  SeriesAbstrPtr T3(unsigned int period, double vFactor) const override;
  SeriesAbstrPtr TEMA(unsigned int period) const override;
  SeriesAbstrPtr TRIMA(unsigned int period) const override;
  SeriesAbstrPtr StochRSIFastK(int period, int fastKPeriod, int fastDPeriod, MAType fastDMAType) const override;
  SeriesAbstrPtr StochRSIFastD(int period, int fastKPeriod, int fastDPeriod, MAType fastDMAType) const override;
  SeriesAbstrPtr Sin() const override;
  SeriesAbstrPtr Cos() const override;
  SeriesAbstrPtr Tan() const override;
  SeriesAbstrPtr Cosh() const override;
  SeriesAbstrPtr Sinh() const override;
  SeriesAbstrPtr Tanh() const override;
  SeriesAbstrPtr Acos() const override;
  SeriesAbstrPtr Asin() const override;
  SeriesAbstrPtr Atan() const override;
  SeriesAbstrPtr Ceil() const override;
  SeriesAbstrPtr Floor() const override;
  SeriesAbstrPtr Exp() const override;
  SeriesAbstrPtr Sqrt() const override;
  SeriesAbstrPtr Ln() const override;
  SeriesAbstrPtr Log10() const override;
};
