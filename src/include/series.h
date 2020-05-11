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
 *  \brief Contains data series related classes and defintions
 */

#ifdef SIMLIB_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

#include "misc.h"
#include "synchronizer.h"

/* @cond */
namespace tradery {
/* @endcond */

// this is to avoid multiple definitions in SIMLIB
enum MAType {
  SMA = 0,
  EMA = 1,
  WMA = 2,
  DEMA = 3,
  TTEMA = 4,
  TTRIMA = 5,
  KAMA = 6,
  MAMA = 7,
  TT3 = 8
};

class Series;

class SeriesIndicatorsAbstr {
 public:
  virtual ~SeriesIndicatorsAbstr() {}
  virtual const Series Min(unsigned int period) const = 0;
  virtual const Series Max(unsigned int period) const = 0;
  virtual bool crossOver(size_t index, const Series& series) const = 0;
  virtual bool crossOver(size_t index, double d) const = 0;
  virtual bool crossUnder(size_t index, const Series& series) const = 0;
  virtual bool crossUnder(size_t index, double d) const = 0;
  virtual bool turnDown(size_t index) const = 0;
  virtual bool turnUp(size_t index) const = 0;
  virtual const Series shiftRight(size_t n) const = 0;
  virtual const Series shiftLeft(size_t n) const = 0;
  virtual const Series SMA(unsigned int period) const = 0;
  virtual const Series EMA(unsigned int period) const = 0;
  virtual const Series EMA(unsigned int period, double exp) const = 0;
  virtual const Series WMA(unsigned int period) const = 0;
  virtual const Series AroonDown(unsigned int period) const = 0;
  virtual const Series AroonUp(unsigned int period) const = 0;
  virtual const Series ROC(unsigned int period) const = 0;
  virtual const Series BBandUpper(unsigned int period, double stdDev) const = 0;
  virtual const Series BBandLower(unsigned int period, double stdDev) const = 0;
  virtual const Series DEMA(unsigned int period) const = 0;
  virtual const Series HTTrendline() const = 0;
  virtual const Series KAMA(unsigned int period) const = 0;
  virtual const Series MAMA(double fastLimit, double slowLimit) const = 0;
  virtual const Series FAMA(double fastLimit, double slowLimit) const = 0;
  virtual const Series MidPoint(unsigned int period) const = 0;
  virtual const Series PPO(unsigned int fastPeriod, unsigned int slowPeriod, MAType maType) const = 0;
  virtual const Series ROCP(unsigned int period) const = 0;
  virtual const Series ROCR(unsigned int period) const = 0;
  virtual const Series ROCR100(unsigned int period) const = 0;
  virtual const Series RSI(unsigned int period) const = 0;
  virtual const Series TRIX(unsigned int period) const = 0;
  virtual const Series HTDCPeriod() const = 0;
  virtual const Series HTDCPhase() const = 0;
  virtual const Series HTPhasorPhase() const = 0;
  virtual const Series HTPhasorQuadrature() const = 0;
  virtual const Series HTSine() const = 0;
  virtual const Series HTLeadSine() const = 0;
  virtual const Series HTTrendMode() const = 0;
  virtual const Series LinearReg(unsigned int period) const = 0;
  virtual const Series LinearRegSlope(unsigned int period) const = 0;
  virtual const Series LinearRegAngle(unsigned int period) const = 0;
  virtual const Series LinearRegIntercept(unsigned int period) const = 0;
  virtual const Series StdDev(unsigned int period, double nbDev) const = 0;
  virtual const Series Variance(unsigned int period, double nbDev) const = 0;
  virtual const Series MOM(unsigned int period) const = 0;
  virtual const Series Momentum(unsigned int period) const = 0;
  virtual const Series MACD(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const = 0;
  virtual const Series MACDSignal(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const = 0;
  virtual const Series MACDHist(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const = 0;
  virtual const Series MACDExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const = 0;
  virtual const Series MACDSignalExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod,
                                     MAType signalMAType) const = 0; virtual const Series MACDHistExt(unsigned int fastPeriod, MAType fastMAType,
                                   unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const = 0;
  virtual const Series MACDFix(unsigned int period) const = 0;
  virtual const Series MACDSignalFix(unsigned int period) const = 0;
  virtual const Series MACDHistFix(unsigned int period) const = 0;
  virtual const Series APO(unsigned int fastPeriod, unsigned int slowPeriod, MAType maType) const = 0;
  virtual const Series T3(unsigned int period, double vFactor) const = 0;
  virtual const Series TEMA(unsigned int period) const = 0;
  virtual const Series TRIMA(unsigned int period) const = 0;
  virtual const Series StochRSIFastK(int period, int fastKPeriod, int fastDPeriod, MAType fastDMAType) const = 0;
  virtual const Series StochRSIFastD(int period, int fastKPeriod, int fastDPeriod, MAType fastDMAType) const = 0;
  virtual const Series Sin() const = 0;
  virtual const Series Cos() const = 0;
  virtual const Series Tan() const = 0;
  virtual const Series Cosh() const = 0;
  virtual const Series Sinh() const = 0;
  virtual const Series Tanh() const = 0;
  virtual const Series Acos() const = 0;
  virtual const Series Asin() const = 0;
  virtual const Series Atan() const = 0;
  virtual const Series Ceil() const = 0;
  virtual const Series Floor() const = 0;
  virtual const Series Exp() const = 0;
  virtual const Series Sqrt() const = 0;
  virtual const Series Ln() const = 0;
  virtual const Series Log10() const = 0;
};

class SeriesAbstr;
using SeriesAbstrPtr = std::shared_ptr<SeriesAbstr>;

class CORE_API SeriesHelper {
 private:
  SeriesAbstrPtr _s;
  size_t _ix;

 public:
  SeriesHelper(SeriesAbstrPtr s, size_t ix);
  double operator=(double d);
  operator double() const;
};

class SeriesAbstr {
 public:
  virtual ~SeriesAbstr() {}
  virtual bool isSynchronized() const = 0;
  virtual Synchronizer::SynchronizerPtr synchronizer() const = 0;
  virtual void synchronize(Synchronizer::SynchronizerPtr synchronizer) = 0;
  static SeriesAbstrPtr create();
  static SeriesAbstrPtr create(size_t size);
  virtual SeriesAbstrPtr clone() const = 0;
  double operator[](size_t index) const { return getValue(index); }
  virtual double setValue(size_t index, double value) = 0;

  virtual double getValue(size_t index) const = 0;
  virtual double& getRef(size_t index) = 0;
  virtual size_t unsyncSize() const = 0;
  virtual size_t size() const = 0;
  virtual void push_back(double value) = 0;
  virtual const double* getArray() const = 0;
  virtual const std::vector<double>& getVector() const = 0;

  virtual SeriesAbstr& operator=(SeriesAbstrPtr series) = 0;
  virtual SeriesAbstrPtr operator*(SeriesAbstrPtr series) const = 0;
  virtual SeriesAbstr& operator*=(SeriesAbstrPtr series) = 0;
  virtual SeriesAbstrPtr operator*(double value) const = 0;
  virtual SeriesAbstr& operator*=(double value) = 0;
  virtual SeriesAbstrPtr operator+(SeriesAbstrPtr series) const = 0;
  virtual SeriesAbstr& operator+=(SeriesAbstrPtr series)  = 0;
  virtual SeriesAbstrPtr operator+(double value) const = 0;
  virtual SeriesAbstr& operator+=(double value) = 0;
  virtual SeriesAbstrPtr operator-(SeriesAbstrPtr series) const = 0;
  virtual SeriesAbstr& operator-=(SeriesAbstrPtr series) = 0;
  virtual SeriesAbstrPtr operator-(double value) const = 0;
  virtual SeriesAbstr& operator-=(double value) = 0;
  virtual SeriesAbstrPtr operator/(SeriesAbstrPtr series) const = 0;
  virtual SeriesAbstr& operator/=(SeriesAbstrPtr series) = 0;
  virtual SeriesAbstrPtr operator/(double value) const = 0;
  virtual SeriesAbstr& operator/=(double value) = 0;
  virtual SeriesAbstrPtr Min(unsigned int period) const = 0;
  virtual SeriesAbstrPtr Max(unsigned int period) const = 0;
  virtual SeriesAbstrPtr MinIndex(unsigned int period) const = 0;
  virtual SeriesAbstrPtr MaxIndex(unsigned int period) const = 0;
  virtual SeriesAbstrPtr multiply(SeriesAbstrPtr series) const = 0;
  virtual SeriesAbstrPtr multiply(double value) const = 0;
  virtual SeriesAbstrPtr add(SeriesAbstrPtr series) const = 0;
  virtual SeriesAbstrPtr add(double value) const = 0;
  virtual SeriesAbstrPtr subtract(SeriesAbstrPtr series) const = 0;
  virtual SeriesAbstrPtr subtract(double value) const = 0;
  virtual SeriesAbstrPtr subtractFrom(double value) const = 0;
  virtual SeriesAbstrPtr divide(SeriesAbstrPtr series) const = 0;
  virtual SeriesAbstrPtr divide(double value) const = 0;
  virtual SeriesAbstrPtr divideBy(double value) const = 0;

  virtual bool crossOver(size_t index, SeriesAbstrPtr series) const = 0;
  virtual bool crossOver(size_t index, double d) const = 0;
  virtual bool crossUnder(size_t index, SeriesAbstrPtr series) const = 0;
  virtual bool crossUnder(size_t index, double d) const = 0;
  virtual bool turnDown(size_t index) const = 0;
  virtual bool turnUp(size_t index) const = 0;

  virtual SeriesAbstrPtr SMA(unsigned int period) const = 0;
  virtual SeriesAbstrPtr EMA(unsigned int period) const = 0;
  virtual SeriesAbstrPtr EMA(unsigned int period, double exp) const = 0;
  virtual SeriesAbstrPtr WMA(unsigned int period) const = 0;
  virtual SeriesAbstrPtr AroonDown(unsigned int period) const = 0;
  virtual SeriesAbstrPtr AroonUp(unsigned int period) const = 0;
  virtual SeriesAbstrPtr ROC(unsigned int period) const = 0;
  virtual SeriesAbstrPtr BBandUpper(unsigned int period, double stdDev) const = 0;
  virtual SeriesAbstrPtr BBandLower(unsigned int period, double stdDev) const = 0;
  virtual SeriesAbstrPtr DEMA(unsigned int period) const = 0;
  virtual SeriesAbstrPtr HTTrendline() const = 0;
  virtual SeriesAbstrPtr KAMA(unsigned int period) const = 0;
  virtual SeriesAbstrPtr MAMA(double fastLimit, double slowLimit) const = 0;
  virtual SeriesAbstrPtr FAMA(double fastLimit, double slowLimit) const = 0;
  virtual SeriesAbstrPtr MidPoint(unsigned int period) const = 0;
  virtual SeriesAbstrPtr PPO(unsigned int fastPeriod, unsigned int slowPeriod, MAType maType) const = 0;
  virtual SeriesAbstrPtr ROCP(unsigned int period) const = 0;
  virtual SeriesAbstrPtr ROCR(unsigned int period) const = 0;
  virtual SeriesAbstrPtr ROCR100(unsigned int period) const = 0;
  virtual SeriesAbstrPtr RSI(unsigned int period) const = 0;
  virtual SeriesAbstrPtr TRIX(unsigned int period) const = 0;
  virtual SeriesAbstrPtr HTDCPeriod() const = 0;
  virtual SeriesAbstrPtr HTDCPhase() const = 0;
  virtual SeriesAbstrPtr HTPhasorPhase() const = 0;
  virtual SeriesAbstrPtr HTPhasorQuadrature() const = 0;
  virtual SeriesAbstrPtr HTSine() const = 0;
  virtual SeriesAbstrPtr HTLeadSine() const = 0;
  virtual SeriesAbstrPtr HTTrendMode() const = 0;
  virtual SeriesAbstrPtr LinearReg(unsigned int period) const = 0;
  virtual SeriesAbstrPtr LinearRegSlope(unsigned int period) const = 0;
  virtual SeriesAbstrPtr LinearRegAngle(unsigned int period) const = 0;
  virtual SeriesAbstrPtr LinearRegIntercept(unsigned int period) const = 0;
  virtual SeriesAbstrPtr StdDev(unsigned int period, double nbDev) const = 0;
  virtual SeriesAbstrPtr Variance(unsigned int period, double nbDev) const = 0;
  virtual SeriesAbstrPtr Correlation(SeriesAbstrPtr series, unsigned int period) const = 0;
  virtual SeriesAbstrPtr Beta(SeriesAbstrPtr series, unsigned int period) const = 0;
  virtual SeriesAbstrPtr TSF(unsigned int period) const = 0;
  virtual SeriesAbstrPtr CMO(unsigned int period) const = 0;
  virtual SeriesAbstrPtr MOM(unsigned int period) const = 0;
  virtual SeriesAbstrPtr Momentum(unsigned int period) const = 0;
  virtual SeriesAbstrPtr MACD(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const = 0;
  virtual SeriesAbstrPtr MACDSignal(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const = 0;
  virtual SeriesAbstrPtr MACDHist(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const = 0;
  virtual SeriesAbstrPtr MACDExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const = 0;
  virtual SeriesAbstrPtr MACDSignalExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const = 0;
  virtual SeriesAbstrPtr MACDHistExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const = 0;
  virtual SeriesAbstrPtr MACDFix(unsigned int period) const = 0;
  virtual SeriesAbstrPtr MACDSignalFix(unsigned int period) const = 0;
  virtual SeriesAbstrPtr MACDHistFix(unsigned int period) const = 0;
  virtual SeriesAbstrPtr APO(unsigned int fastPeriod, unsigned int slowPeriod, MAType maType) const = 0;
  virtual SeriesAbstrPtr T3(unsigned int period, double vFactor) const = 0;
  virtual SeriesAbstrPtr TEMA(unsigned int period) const = 0;
  virtual SeriesAbstrPtr TRIMA(unsigned int period) const = 0;
  virtual SeriesAbstrPtr StochRSIFastK(int period, int fastKPeriod, int fastDPeriod, MAType fastDMAType) const = 0;
  virtual SeriesAbstrPtr StochRSIFastD(int period, int fastKPeriod, int fastDPeriod, MAType fastDMAType) const = 0;
  virtual SeriesAbstrPtr shiftRight(size_t n) const = 0;
  virtual SeriesAbstrPtr shiftLeft(size_t n) const = 0;
  virtual SeriesAbstrPtr Sin() const = 0;
  virtual SeriesAbstrPtr Cos() const = 0;
  virtual SeriesAbstrPtr Tan() const = 0;
  virtual SeriesAbstrPtr Cosh() const = 0;
  virtual SeriesAbstrPtr Sinh() const = 0;
  virtual SeriesAbstrPtr Tanh() const = 0;
  virtual SeriesAbstrPtr Acos() const = 0;
  virtual SeriesAbstrPtr Asin() const = 0;
  virtual SeriesAbstrPtr Atan() const = 0;
  virtual SeriesAbstrPtr Ceil() const = 0;
  virtual SeriesAbstrPtr Floor() const = 0;
  virtual SeriesAbstrPtr Exp() const = 0;
  virtual SeriesAbstrPtr Sqrt() const = 0;
  virtual SeriesAbstrPtr Ln() const = 0;
  virtual SeriesAbstrPtr Log10() const = 0;
};

/**
 * Interface (abstract class) for a Series of double values.
 *
 * The user does not need to create concrete classes derived from
 * Series, instead they will use the create methods to create
 * Series objects.
 */
class CORE_API Series : public SeriesIndicatorsAbstr {
 private:
  SeriesAbstrPtr _series;

 public:
  Series(SeriesAbstrPtr series) : _series(series) {}

 public:
  Series() : _series(SeriesAbstr::create()) {}

  Series(size_t size) : _series(SeriesAbstr::create(size)) {}

  Series(const Series& series) : _series(series._series) {}

  virtual Synchronizer::SynchronizerPtr synchronizer() const {
    return _series->synchronizer();
  }
  virtual bool isSynchronized() const { return _series->isSynchronized(); }
  virtual void synchronize(Synchronizer::SynchronizerPtr synchronizer) {
    _series->synchronize(synchronizer);
  }

  virtual size_t unsyncSize() const { return _series->unsyncSize(); }
  virtual size_t size() const { return _series->size(); }

  //  double getValue( size_t index ) const throw
  //  (SeriesIndexOutOfRangeException) { return _series->getValue( index ); }
  double operator[](size_t index) const {
    return _series->getValue(index);
  }

  /**
   * Operator [] for retrieving and/or changing the value of an element in a
   * non const Series, for array like operations on Series
   *
   * This operator returns a non const reference to an element of a series
   * allowing it to be on the left side of an assigment operator
   *
   * @param index  The index whose non const reference is to be returned
   *
   * @return A non const reference to an element of a Series
   * @exception SeriesIndexOutOfRangeException
   *                   Thrown if index points outside of the series
   */

  SeriesHelper operator[](size_t index) {
    return SeriesHelper(_series, index);
  }

  double setValue(size_t index, double value) {
    return _series->setValue(index, value);
  }
  const double* getArray() const { return _series->getArray(); }

  const std::vector<double>& getVector() const { return _series->getVector(); }

  const SeriesAbstr& getSeries() const {
    assert(_series);
    return *_series;
  }
  /**
   * Returns a reference to an element in a series, allowint it to
   * be read and/or modified
   *
   * @param index  the index of the element whose reference is to be returned
   *
   * @return non const reference to an element in the series
   * @exception SeriesIndexOutOfRangeException
   *                   Thrown if index points outside of the Series
   */
  //  virtual double& getValue( size_t index ) throw
  //  (SeriesIndexOutOfRangeException) { return _series->getValue( index ); }

  /**
   * Sets a value of an element in a non const series
   *
   * Note: use the operator[] whenver possible as it usually makes the
   * code clearer
   *
   * @param index  The index of the element to be set in the Series
   * @param value
   */
  /*  virtual void setValue( size_t index, double value )
    {
    }
  */
  /**
   * Addes an element to the end of the series
   *
   * <B>Not thread safe</B>
   *
   * @param value  The value to be added to the end of the series
   */
  virtual void push_back(double value) { _series->push_back(value); }

  /**
   * \brief Series assignment operator
   *
   * copies all values from a Series into the current Series
   *
   * @param series The series whose values are to be copied into the current
   * series
   *
   * @return
   * @exception OperationOnUnequalSizeSeriesException
   *                   Thrown if the two series are of different sizes
   */
  virtual Series& operator=(const Series& series) {
    if (this != &series) {
      _series = series._series;
    }
    return *this;
  }
  /**\addtogroup Indicators
   *
   * Technical indicators
   *
   * @{
   */

  /** \defgroup SeriesIndicators Technical Indicators on Series
   @{
   */
  /**\name Arithmetic Operations on Series
   * @{
   */
  /**
   * \brief Multiplies the values in the current series with the
   * corresponding values in another Series. The result is stored in a new
   * series.
   *
   * The two series must be of equal size, or an exception of type
   * OperationOnUnequalSeriesException will be thrown
   *
   * The new series is returned as a SeriesConstPtr which doesn't need to be
   * deleted explicitely by the user
   *
   * Example:
   * \code
   * // create 2 series of equal size
   * SeriesPtr series1 = Series::create( 10000 );
   * SeriesPtr series2 = Series::create( 10000 );
   * // populate the 2 series
   *
   * // performa the multiplication
   * // note: the two pointers must be dereferenced first hence the parantheses
   * and * operator
   *
   * ConstSeriesPtr result = series1->multiply( *series2 );
   *
   * //result contains the result
   *
   * \endcode
   *
   * @param series The other operand of the multiplication operation
   *
   * @return The new series containing the result of the multiplication
   * @exception OperationOnUnequalSizeSeriesException
   *                   thrown if index is outside the Series
   */
  /**
   * Multiplication operator - multiplies the values in the current series with
   * the corresponding values in a differen series. The result is stored in a
   * new series
   *
   * The two series must be of equal size, or an exception of type
   * OperationOnUnequalSeriesException will be throw
   *
   * The new series is returned as a SeriesConstPtr which doesn't need to be
   * deleted explicitely by the user
   *
   * @param series The other operand of the multiplication operation
   *
   * @return The new series containing the result of the multiplication
   * @exception OperationOnUnequalSizeSeriesException
   *                   thrown if index is outside the Series
   */
  Series operator*(const Series& series) const {
    return Series((*_series) * series._series);
  }
  /**
   * Multiplication operator - multiplies the values in the current series with
   * the corresponding values in a differen series and stores the result in the
   * current series
   *
   * The two series must be of equal size, or an exception of type
   * OperationOnUnequalSeriesException will be throw
   *
   * @param series The other operand of the multiplication operation
   *
   * @return A reference to the current Series, to allow chaining of operations
   * @exception OperationOnUnequalSizeSeriesException
   *                   thrown if index is outside the Series
   */
  Series& operator*=(const Series& series) {
    (*_series) *= series._series;
    return *this;
  }
  /**
   * \brief Multiplication operator - multiplies the elements of the current
   * series with a constant value
   *
   * All values in the current series are multiplied with the constant, and the
   * result is stored in a new series.
   *
   * The new series is returned as a SeriesConstPtr which doesn't need to be
   * deleted
   *
   * @param value  The constant with which to multiply the series
   * @return The new series containing the result of the multiplication
   * @exception OperationOnUnequalSizeSeriesException
   */
  Series operator*(double value) const { return Series((*_series) * value); }
  /**
   * \brief Multiplication operator - multiplies the elements of the current
   * series with a constant value and stores the result in the current series
   *
   *
   * A reference to the current series is returned to allow chaining of
   * operations
   *
   * @param value  The constant with which to multiply the series
   * @return The new series containing the result of the multiplication
   * @exception OperationOnUnequalSizeSeriesException
   */
  Series& operator*=(double value) {
    (*_series) *= value;
    return *this;
  }
  /**
   * Addition operator - adds the values in the current series with the
   * corresponding values in a differen series. The result is stored in a new
   * series
   *
   * The two series must be of equal size, or an exception of type
   * OperationOnUnequalSeriesException will be throw
   *
   * The new series is returned as a SeriesConstPtr which doesn't need to be
   * deleted explicitely by the user
   *
   * @param series The other operand of the addition operation
   *
   * @return The new series containing the result of the addition
   * @exception OperationOnUnequalSizeSeriesException
   *                   thrown if index is outside the Series
   */
  Series operator+(const Series& series) const {
    return Series((*_series) + series._series);
  }
  /**
   * Addition operator - adds the values in the current series with the
   * corresponding values in a differen series and stores the result in the
   * current series
   *
   * The two series must be of equal size, or an exception of type
   * OperationOnUnequalSeriesException will be throw
   *
   * @param series The other operand of the addition operation
   *
   * @return A reference to the current Series, to allow chaining of operations
   * @exception OperationOnUnequalSizeSeriesException
   *                   thrown if index is outside the Series
   */
  Series& operator+=(const Series& series) {
    (*_series) += series._series;
    return *this;
  }
  /**
   * \brief Addition operator - adds the elements of the current series with a
   * constant value
   *
   * All values in the current series are summed with the constant, and the
   * result is stored in a new series.
   *
   * The new series is returned as a SeriesConstPtr which doesn't need to be
   * deleted
   *
   * Example:
   * \code
   * // create 2 series of equal size
   * SeriesPtr series1 = Series::create( 10000 );
   * SeriesPtr series2 = Series::create( 10000 );
   *
   * Series& s1 = *series1;
   * Series& s2 = *series2;
   *
   * // populate the 2 series
   *
   * // performa the addition
   *
   * SeriesConstPtr result = s1 + s2;
   *
   * //result contains the result
   *
   * \endcode
   *
   * @param value  The constant with which to add the series
   * @return The new series containing the result of the add
   * @exception OperationOnUnequalSizeSeriesException
   */
  Series operator+(double value) const { return Series((*_series) + value); }
  /**
   * \brief Addition operator - adds the elements of the current series with a
   * constant value and stores the result in the current series
   *
   *
   * A reference to the current series is returned to allow chaining of
   * operations
   *
   * Example:
   * \code
   * // create 2 series of equal size
   * SeriesPtr series1 = Series::create( 10000 );
   * SeriesPtr series2 = Series::create( 10000 );
   *
   * Series& s1 = *series1;
   * Series& s2 = *series2;
   *
   * // populate the 2 series
   *
   * // performa the addition
   *
   * s1 += s2;
   *
   * //series1 contains the result
   *
   * \endcode
   *
   * @param value  The constant with which to add the series
   * @return The new series containing the result of the addition
   * @exception OperationOnUnequalSizeSeriesException
   */
  Series& operator+=(double value) {
    (*_series) += value;
    return *this;
  }
  /**
   * \brief Subtraction operator - subtracts the elements of a series from the
   * elements of the current series and stores the result into a new series
   *
   * The two series must be of equal sizes or an
   * OperationOnUnequalSizeSeriesException will be thrown
   *
   * Example:
   * \code
   * // create 2 series of equal size
   * SeriesPtr series1 = Series::create( 10000 );
   * SeriesPtr series2 = Series::create( 10000 );
   * // populate the 2 series
   *
   * // performa the subtraction
   * // note: the two pointers must be dereferenced first hence the parantheses
   * and * operator
   *
   * SeriesConstPtr result = (*series1) - (*series2);
   *
   * //series1 contains the result
   *
   * \endcode
   *
   * @param series The series to subtract from the current series
   * @return The new series containing the result of the subtraction
   * @exception OperationOnUnequalSizeSeriesException
   */
  Series operator-(const Series& series) const {
    return Series((*_series) - series._series);
  }
  /**
   * \brief Subtraction operator -= subtracts the elements of a series from the
   * elements of the current series and stores the result in the current series
   *
   *
   * A reference to the current series is returned to allow chaining of
   * operations
   *
   * Example:
   * \code
   * // create 2 series of equal size
   * SeriesPtr series1 = Series::create( 10000 );
   * SeriesPtr series2 = Series::create( 10000 );
   * // populate the 2 series
   *
   * // performa the subtraction
   * // note: the two pointers must be dereferenced first hence the parantheses
   * and * operator
   *
   * (*series1) += (*series2);
   *
   * //series1 contains the result
   *
   * \endcode
   *
   * @param series The Series to subtract from the current series
   * @return The new series containing the result of the subtraction
   * @exception OperationOnUnequalSizeSeriesException
   */
  Series& operator-=(const Series& series) {
    (*_series) -= series._series;
    return *this;
  }
  /**
   * \brief Subtraction operator - subtracts a value from the elements of the
   * current series
   *
   * Creates a new series whose elements are the result of subtracting the value
   * from the elements of the current series.
   *
   * The new series is returned as a SeriesConstPtr which doesn't need to be
   * deleted by the user
   *
   * Example:
   * \code
   * // create a series
   * SeriesPtr series = Series::create( 10000 );
   * double value = 15.25;
   *
   * Series& s = *series1;
   *
   * // populate the series
   *
   * // performa the subtraction
   *
   * SeriesConstPtr result = s - value;
   *
   * //result contains the result
   *
   * \endcode
   *
   * @param value  The constant to subtract from the series
   * @return The new series containing the result of the subtraction
   * @exception OperationOnUnequalSizeSeriesException
   */
  Series operator-(double value) const { return Series((*_series) - value); }
  /**
   * \brief Subtraction operator -= sbutracts a value from the elements of the
   * current series and stores the result in the current series
   *
   * Example:
   * \code
   * // create a series
   * SeriesPtr series = Series::create( 10000 );
   * double value = 15.25;
   *
   * Series& s = *series1;
   *
   * // populate the series
   *
   * // performa the subtraction
   *
   * s -= value;
   *
   * //series contains the result
   *
   * \endcode
   *
   * @param value  The constant to subtract from the series
   * @return The new series containing the result of the subtraction
   * @exception OperationOnUnequalSizeSeriesException
   */
  Series& operator-=(double value) {
    (*_series) -= value;
    return *this;
  }
  /**
   * \brief Operator / - divides the current Series by the corresponding values
   * in second operand Series. The result is stored in a new series.
   *
   * The two series must be of equal size, or an exception of type
   * OperationOnUnequalSeriesException will be thrown
   *
   * The new series is returned as a SeriesConstPtr which doesn't need to be
   * deleted explicitely by the user
   *
   * example
   *
   * \code
   * // create 2 series of equal size
   * SeriesPtr series1 = Series::create( 10000 );
   * SeriesPtr series2 = Series::create( 10000 );
   * // populate the 2 series
   *
   * // performs the division series1/series2
   * Series& s1 = *series1;
   * Series& s2 = *series2;
   *
   * SeriesConstPtr result = s1 / s2;
   *
   * //result contains the result
   *
   * \endcode
   *
   * @param series The other operand of the division operation
   *
   * @return The new series containing the result of the division operation
   * @exception OperationOnUnequalSizeSeriesException
   *                   thrown if the two series are of different sizes.
   * @exception DivideByZeroException
   *                   Thrown if the argument series has one or more 0 elements
   */
  Series operator/(const Series& series) const {
    return Series((*_series) / series._series);
  }
  /**
   * \brief Operator /= divides the current Series by the corresponding values
   * in second operand Series. The result is stored in the current series
   *
   * The two series must be of equal size, or an exception of type
   * OperationOnUnequalSeriesException will be thrown
   *
   * example
   *
   * \code
   * // create 2 series of equal size
   * SeriesPtr series1 = Series::create( 10000 );
   * SeriesPtr series2 = Series::create( 10000 );
   * // populate the 2 series
   *
   * // performs the division series1/series2
   * Series& s1 = *series1;
   * Series& s2 = *series2;
   *
   * s1 /= s2;
   *
   * //s1 contains the result
   *
   * \endcode
   *
   * @param series The other operand of the division operation
   *
   * @return A reference to the current series, to allow chaining of operations
   * @exception OperationOnUnequalSizeSeriesException
   *                   thrown if the two series are of different sizes.
   * @exception DivideByZeroException
   *                   Thrown if the argument series has one or more 0 elements
   */
  virtual Series& operator/=(const Series& series) {
    (*_series) /= series._series;
    return *this;
  }
  /**
   * \brief divides the current Series by a value. The result is stored in a new
   * series
   *
   * The result series does not need to be deleted by the user
   *
   * example
   *
   * \code
   * // create a series
   * SeriesPtr series = Series::create( 10000 );
   * // populate the series
   *
   * // performs the division series1/series2
   * Series& s = *series;
   * const double value = 15.25;
   * SeriesConstPtr result = s.divide( value );
   *
   * // result contains the result of the division
   *
   * \endcode
   *
   * @param value The value operand of the division operation
   *
   * @return A reference to the current series, to allow chaining of operations
   * @exception DivideByZeroException
   *                   Thrown if the value is 0
   */
  Series operator/(double value) const {
    return Series((*_series) / value);
  }
  /**
   * \brief Operator /= divides the current Series by a value. The result is
   * stored in the current series
   *
   * example
   *
   * \code
   * // create a series
   * SeriesPtr series = Series::create( 10000 );
   * // populate the series
   *
   * // performs the division series1/series2
   * Series& s = *series;
   * const double value = 15.25;
   * s /= value;
   *
   * // the current series contains the result
   *
   * \endcode
   *
   * @param value The value operand of the division operation
   *
   * @return A reference to the current series, to allow chaining of operations
   * @exception DivideByZeroException
   *                   Thrown if the value is 0
   */
  Series& operator/=(double value) {
    (*_series) /= value;
    return *this;
  }
  /**
   * Lowest value over a specified period
   */
  const Series Min(unsigned int period) const override {
    return Series(_series->Min(period));
  }
  /**
   * Highest value over a specified period
   */
  const Series Max(unsigned int period) const override {
    return Series(_series->Max(period));
  }
  //@}
  virtual const Series MinIndex(unsigned int period) const {
    return Series(_series->MinIndex(period));
  }
  /**
   * Highest value over a specified period
   */
  virtual const Series MaxIndex(unsigned int period) const {
    return Series(_series->MaxIndex(period));
  }
  /**
   * Indicates if the current series crosses over a different series at the
   * current index
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: define crossover </FONT></FONT>
   * \endhtmlonly -->
   *
   * @param index  The index at which the crossover is to be tested
   * @param series The second series
   * @return true if crossed over, false if not
   * @exception OperationOnUnequalSizeSeriesException
   *                   Thrown if the two series are of different sizes
   * @exception SeriesIndexOutOfRangeException
   *                   Thrown if the index is outside of either of the Series
   */
  bool crossOver(size_t index, const Series& series) const override {
    return _series->crossOver(index, series._series);
  }
  /**
   * Indicates if the current series crosses over a value at the current index
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: define crossover </FONT>
   * \endhtmlonly -->
   *
   * @param index  The index at which the crossover is to be tested
   * @param d      The value to be tested for crossover
   * @return true if crossed over, false if not
   * @exception SeriesIndexOutOfRangeException
   *                   Thrown if the index is outside of either of the Series
   */
  bool crossOver(size_t index, double d) const override {
    return _series->crossOver(index, d);
  }
  /**
   * Indicates if the current series crosses under a different series at the
   * current index
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: define crossunder </FONT>
   * \endhtmlonly -->
   *
   * @param index  The index at which the crossunder is to be tested
   * @param series The second series
   * @return true if crossed under, false if not
   * @exception OperationOnUnequalSizeSeriesException
   *                   Thrown if the two series are of different sizes
   * @exception SeriesIndexOutOfRangeException
   *                   Thrown if the index is outside of either of the Series
   */
  bool crossUnder(size_t index, const Series& series) const override {
    return _series->crossUnder(index, series._series);
  }
  /**
   * Indicates if the current series crosses under a value at the current index
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: define crossunder </FONT>
   * \endhtmlonly -->
   *
   * @param index  The index at which the crossunder is to be tested
   * @param d      The value to be tested for crossunder
   * @return true if crossed under, false if not
   * @exception SeriesIndexOutOfRangeException
   *                   Thrown if the index is outside of either of the Series
   */
  bool crossUnder(size_t index, double d) const override {
    return _series->crossUnder(index, d);
  }
  /**
   * Indicates if the series has turned down at the current index
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: define turn down </FONT>
   * \endhtmlonly -->
   *
   * @param index  The index at which the turn down is to be checked
   * @return true if turned down, false if not
   * @exception SeriesIndexOutOfRangeException
   *                   Thrown if the index is outside the series range
   */
  bool turnDown(size_t index) const override {
    return _series->turnDown(index);
  }
  /**
   * Indicates if the series has turned up at the current index
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: define turn up </FONT> \endhtmlonly
   * -->
   *
   * @param index  The index at which the turn up is to be checked
   * @return true if turned up, false if not
   * @exception SeriesIndexOutOfRangeException
   *                   Thrown if the index is outside the series range
   */
  bool turnUp(size_t index) const override {
    return _series->turnUp(index);
  }
  const Series shiftRight(size_t n) const override {
    return Series(_series->shiftRight(n));
  }
  const Series shiftLeft(size_t n) const override {
    return Series(_series->shiftLeft(n));
  }

  const Series SMA(unsigned int period) const override {
    return Series(_series->SMA(period));
  }
  const Series EMA(unsigned int period) const override {
    return Series(_series->EMA(period));
  }
  const Series EMA(unsigned int period, double exp) const override {
    return Series(_series->EMA(period, exp));
  }
  const Series WMA(unsigned int period) const override {
    return Series(_series->WMA(period));
  }
  const Series AroonDown(unsigned int period) const override {
    return Series(_series->AroonDown(period));
  }
  const Series AroonUp(unsigned int period) const override {
    return Series(_series->AroonUp(period));
  }
  //  SeriesConstPtr getAroonOscillatorSeries( const Series& series,
  //  unsigned int period ) const = 0;
  const Series ROC(unsigned int period) const override {
    return Series(_series->ROC(period));
  }
  //  SeriesConstPtr getMACDFixSeries( const Series& series ) const = 0;
  const Series BBandUpper(unsigned int period, double stdDev) const override {
    return Series(_series->BBandUpper(period, stdDev));
  }
  const Series BBandLower(unsigned int period, double stdDev) const override {
    return Series(_series->BBandLower(period, stdDev));
  }
  const Series DEMA(unsigned int period) const override {
    return Series(_series->DEMA(period));
  }
  const Series HTTrendline() const override {
    return Series(_series->HTTrendline());
  }
  const Series KAMA(unsigned int period) const override {
    return Series(_series->KAMA(period));
  }
  const Series MAMA(double fastLimit, double slowLimit) const override {
    return Series(_series->MAMA(fastLimit, slowLimit));
  }
  const Series FAMA(double fastLimit, double slowLimit) const override {
    return Series(_series->FAMA(fastLimit, slowLimit));
  }
  const Series MidPoint(unsigned int period) const override {
    return Series(_series->MidPoint(period));
  }
  const Series PPO(unsigned int fastPeriod, unsigned int slowPeriod,
                           MAType maType) const override {
    return Series(_series->PPO(fastPeriod, slowPeriod, maType));
  }
  const Series ROCP(unsigned int period) const override {
    return Series(_series->ROCP(period));
  }
  const Series ROCR(unsigned int period) const override {
    return Series(_series->ROCR(period));
  }
  const Series ROCR100(unsigned int period) const override {
    return Series(_series->ROCR100(period));
  }
  const Series RSI(unsigned int period) const override {
    return Series(_series->RSI(period));
  }
  const Series TRIX(unsigned int period) const override {
    return Series(_series->TRIX(period));
  }
  const Series HTDCPeriod() const override {
    return Series(_series->HTDCPeriod());
  }
  const Series HTDCPhase() const override {
    return Series(_series->HTDCPhase());
  }
  const Series HTPhasorPhase() const override {
    return Series(_series->HTPhasorPhase());
  }
  const Series HTPhasorQuadrature() const override {
    return Series(_series->HTPhasorQuadrature());
  }
  const Series HTSine() const override { return Series(_series->HTSine()); }
  const Series HTLeadSine() const override {
    return Series(_series->HTLeadSine());
  }
  const Series HTTrendMode() const override {
    return Series(_series->HTTrendMode());
  }
  const Series LinearReg(unsigned int period) const override {
    return Series(_series->LinearReg(period));
  }
  const Series LinearRegSlope(unsigned int period) const override {
    return Series(_series->LinearRegSlope(period));
  }
  const Series LinearRegAngle(unsigned int period) const override {
    return Series(_series->LinearRegAngle(period));
  }
  const Series LinearRegIntercept(unsigned int period) const override {
    return Series(_series->LinearRegIntercept(period));
  }
  const Series StdDev(unsigned int period, double nbDev) const override {
    return Series(_series->StdDev(period, nbDev));
  }
  const Series Variance(unsigned int period, double nbDev) const override {
    return Series(_series->Variance(period, nbDev));
  }
  virtual const Series Correlation(const Series& series, unsigned int period) const {
    return Series(_series->Correlation(series._series, period));
  }
  virtual const Series Beta(const Series& series, unsigned int period) const {
    return Series(_series->Beta(series._series, period));
  }
  virtual const Series TSF(unsigned int period) const {
    return Series(_series->TSF(period));
  }
  virtual const Series CMO(unsigned int period) const {
    return Series(_series->CMO(period));
  }
  /** Momentum indicator
   */
  const Series MOM(unsigned int period) const override {
    return Series(_series->MOM(period));
  }
  /** Momentum indicator
   */
  const Series Momentum(unsigned int period) const override {
    return Series(_series->Momentum(period));
  }
  const Series MACD(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const override {
    return Series(_series->MACD(fastPeriod, slowPeriod, signalPeriod));
  }
  const Series MACDSignal(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const override {
    return Series(_series->MACDSignal(fastPeriod, slowPeriod, signalPeriod));
  }
  const Series MACDHist(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const override {
    return Series(_series->MACDHist(fastPeriod, slowPeriod, signalPeriod));
  }
  const Series MACDExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const override {
    return Series(_series->MACDExt(fastPeriod, fastMAType, slowPeriod, slowMAType, signalPeriod, signalMAType));
  }
  const Series MACDSignalExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const override {
    return Series(_series->MACDSignalExt(fastPeriod, fastMAType, slowPeriod, slowMAType, signalPeriod, signalMAType));
  }
  const Series MACDHistExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const override {
    return Series(_series->MACDHistExt(fastPeriod, fastMAType, slowPeriod, slowMAType, signalPeriod, signalMAType));
  }
  const Series MACDFix(unsigned int period) const override {
    return Series(_series->MACDFix(period));
  }
  const Series MACDSignalFix(unsigned int period) const override {
    return Series(_series->MACDSignalFix(period));
  }
  const Series MACDHistFix(unsigned int period) const override {
    return Series(_series->MACDHistFix(period));
  }
  const Series APO(unsigned int fastPeriod, unsigned int slowPeriod, MAType maType) const override {
    return Series(_series->APO(fastPeriod, slowPeriod, maType));
  }
  const Series T3(unsigned int period, double vFactor) const override {
    return Series(_series->T3(period, vFactor));
  }
  const Series TEMA(unsigned int period) const override {
    return Series(_series->TEMA(period));
  }
  const Series TRIMA(unsigned int period) const override {
    return Series(_series->TRIMA(period));
  }
  const Series StochRSIFastK(int period, int fastKPeriod, int fastDPeriod, MAType fastDMAType) const override {
    return Series( _series->StochRSIFastK(period, fastKPeriod, fastDPeriod, fastDMAType));
  }
  const Series StochRSIFastD(int period, int fastKPeriod, int fastDPeriod, MAType fastDMAType) const override {
    return Series(_series->StochRSIFastD(period, fastKPeriod, fastDPeriod, fastDMAType));
  }
  const Series Sin() const override { return Series(_series->Sin()); }
  const Series Cos() const override { return Series(_series->Cos()); }
  const Series Tan() const override { return Series(_series->Tan()); }
  const Series Cosh() const override { return Series(_series->Cosh()); }
  const Series Sinh() const override { return Series(_series->Sinh()); }
  const Series Tanh() const override { return Series(_series->Tanh()); }
  const Series Acos() const override { return Series(_series->Acos()); }
  const Series Asin() const override { return Series(_series->Asin()); }
  const Series Atan() const override { return Series(_series->Atan()); }
  const Series Ceil() const override { return Series(_series->Ceil()); }
  const Series Floor() const override { return Series(_series->Floor()); }
  const Series Exp() const override { return Series(_series->Exp()); }
  const Series Sqrt() const override { return Series(_series->Sqrt()); }
  const Series Ln() const override { return Series(_series->Ln()); }
  const Series Log10() const override { return Series(_series->Log10()); }

  //@}
  //@}

  /**
   * Dumps the contents of the series to the output stream
   *
   * <B>Not thread safe</B>
   *
   * @param os     Output stream to dump the contents of the series to
   * @param k      Maximum number of elements to be dumped
   * @return A reference to the output stream
   */
  std::ostream& dump(std::ostream& os, size_t k) const;
  std::ostream& dumpUnsynced(std::ostream& os, size_t k) const;
};

/**
 * Non member operator- enabling subtracting a Series from a value
 *
 * The result is stored in a new Series which doesn't need to be deleted by the
 * user
 *
 * Example:
 * \code
 * // create a series
 * SeriesPtr series = Series::create( 10000 );
 * const double value;
 * // populate the series
 *
 * Series& s = *series;
 * // performa the subtraction
 *
 * SeriesConstPtr result = value - s;
 *
 * //result contains the result
 *
 * \endcode
 *
 * @param value  The first operand of the subtraction
 * @param series The first operand of the subtraction
 *
 * @return A new series containing the result of the subtraction
 */
CORE_API Series operator-(double value, const Series& series);

/**
 * Non member operator+ enabling adding a value to a series
 *
 * The result is stored in a new Series which doesn't need to be deleted by the
 * user
 *
 * Example:
 * \code
 * // create a series
 * SeriesPtr series = Series::create( 10000 );
 * const double value;
 * // populate the series
 *
 * Series& s = *series;
 * // performa the addition
 *
 * SeriesConstPtr result = value + s;
 *
 * //result contains the result
 *
 * \endcode
 *
 * @param value  The first operand of the addition
 * @param series The first operand of the addition
 *
 * @return A new series containing the result of the addition
 */
CORE_API Series operator+(double value, const Series& series);

/**
 * Non member operator* enabling multiplying a value with a Series
 *
 * The result is stored in a new Series which doesn't need to be deleted by the
 * user
 *
 * Example:
 * \code
 * // create a series
 * SeriesPtr series = Series::create( 10000 );
 * const double value;
 * // populate the series
 *
 * Series& s = *series;
 * // performa the multiplication
 *
 * SeriesConstPtr result = value * s;
 *
 * //result contains the result
 *
 * \endcode
 *
 * @param value  The first operand of the multiplication
 * @param series The first operand of the multiplication
 *
 * @return A new series containing the result of the subtraction
 */
CORE_API Series operator*(double value, const Series& series);

/**
 * Non member operator/ enabling dividing a value by a Series
 *
 * The result is stored in a new Series which doesn't need to be deleted by the
 * user
 *
 * Example:
 * \code
 * // create a series
 * SeriesPtr series = Series::create( 10000 );
 * const double value;
 * // populate the series
 *
 * Series& s = *series;
 * // performa the division
 *
 * SeriesConstPtr result = value / s;
 *
 * //result contains the result
 *
 * \endcode
 *
 * @param value  The first operand of the division
 * @param series The first operand of the division
 *
 * @return A new series containing the result of the division
 */
CORE_API inline Series operator/(double value, const Series& series);

}  // namespace tradery
