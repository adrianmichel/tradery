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

#pragma warning(disable : 4786)
#include "stdafx.h"
#include "seriesimpl.h"
#include "bars.h"
#include "indicators.h"

/*
#ifdef _DEBUG
  #undef THIS_FILE
static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
#endif
*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SeriesCache* _cache;

unsigned long EmptySeries::_l = 0;
std::mutex EmptySeries::_mutex;

SeriesAbstrPtr SeriesImpl::shiftRight(size_t n) const {
  return _cache->findAndAdd(MakeShiftRightSeries(*this, n));
}

SeriesAbstrPtr SeriesImpl::shiftLeft(size_t n) const {
  return _cache->findAndAdd(MakeShiftLeftSeries(*this, n));
}

SeriesAbstrPtr SeriesImpl::multiply(SeriesAbstrPtr series) const {
  return _cache->findAndAdd(MakeMultiplySeries(*this, *series));
}

SeriesAbstrPtr SeriesImpl::multiply(double value) const {
  return _cache->findAndAdd(MakeMultiplySeriesByValue(*this, value));
}

SeriesAbstrPtr SeriesImpl::add(SeriesAbstrPtr series) const {
  return _cache->findAndAdd(MakeAddSeries(*this, *series));
}

SeriesAbstrPtr SeriesImpl::add(double value) const {
  return _cache->findAndAdd(MakeAddSeriesToValue(*this, value));
}

SeriesAbstrPtr SeriesImpl::subtract(SeriesAbstrPtr series) const {
  return _cache->findAndAdd(MakeSubtractSeries(*this, *series));
}

SeriesAbstrPtr SeriesImpl::subtract(double value) const {
  return _cache->findAndAdd(MakeSubtractValueFromSeries(*this, value));
}

SeriesAbstrPtr SeriesImpl::divide(double value) const {
  return _cache->findAndAdd(MakeDivideSeriesByValue(*this, value));
}

SeriesAbstrPtr SeriesImpl::divideBy(double value) const {
  return _cache->findAndAdd(MakeDivideValueBySeries(*this, value));
}

SeriesAbstrPtr SeriesImpl::subtractFrom(double value) const {
  return _cache->findAndAdd(MakeSubtractSeriesFromValue(*this, value));
}
SeriesAbstrPtr SeriesImpl::divide(SeriesAbstrPtr series) const {
  return _cache->findAndAdd(MakeDivideSeries(*this, *series));
}

// TODO: handle exceptions thrown inside the Indicators. Necessary?

SeriesAbstrPtr SeriesImpl::SMA(unsigned int period) const {
  return _cache->findAndAdd(MakeSMASeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::EMA(unsigned int period) const {
  return _cache->findAndAdd(MakeFixedExpEMASeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::EMA(unsigned int period, double exp) const {
  return _cache->findAndAdd(MakeEMASeries(*this, period, exp));
}

SeriesAbstrPtr SeriesImpl::WMA(unsigned int period) const {
  return _cache->findAndAdd(MakeWMASeries(*this, period, SMA(period)));
}

SeriesAbstrPtr SeriesImpl::AroonDown(unsigned int period) const {
  return _cache->findAndAdd(MakeAroonDownSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::AroonUp(unsigned int period) const {
  return _cache->findAndAdd(MakeAroonUpSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::RSI(unsigned int period) const {
  return _cache->findAndAdd(MakeRSISeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::ROC(unsigned int period) const {
  return _cache->findAndAdd(MakeROCSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::BBandUpper(unsigned int period,
                                      double stdDev) const {
  return _cache->findAndAdd(MakeBBandUpperSeries(*this, period, stdDev));
}

SeriesAbstrPtr SeriesImpl::BBandLower(unsigned int period,
                                      double stdDev) const {
  return _cache->findAndAdd(MakeBBandLowerSeries(*this, period, stdDev));
}

SeriesAbstrPtr SeriesImpl::DEMA(unsigned int period) const {
  return _cache->findAndAdd(MakeDEMASeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::HTTrendline() const {
  return _cache->findAndAdd(MakeHTTrendlineSeries(*this));
}

SeriesAbstrPtr SeriesImpl::KAMA(unsigned int period) const {
  return _cache->findAndAdd(MakeKAMASeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::MAMA(double fastLimit, double slowLimit) const {
  return _cache->findAndAdd(MakeMAMASeries(*this, fastLimit, slowLimit));
}

SeriesAbstrPtr SeriesImpl::FAMA(double fastLimit, double slowLimit) const {
  return _cache->findAndAdd(MakeFAMASeries(*this, fastLimit, slowLimit));
}

SeriesAbstrPtr SeriesImpl::Min(unsigned int period) const {
  return _cache->findAndAdd(MakeMinSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::Max(unsigned int period) const {
  return _cache->findAndAdd(MakeMaxSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::MaxIndex(unsigned int period) const {
  return _cache->findAndAdd(MakeMaxIndexSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::MinIndex(unsigned int period) const {
  return _cache->findAndAdd(MakeMinIndexSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::MidPoint(unsigned int period) const {
  return _cache->findAndAdd(MakeMidPointSeries(*this, period));
}
SeriesAbstrPtr SeriesImpl::PPO(unsigned int fastPeriod, unsigned int slowPeriod, MAType maType) const {
  return _cache->findAndAdd(MakePPOSeries(*this, fastPeriod, slowPeriod, maType));
}

SeriesAbstrPtr SeriesImpl::ROCP(unsigned int period) const {
  return _cache->findAndAdd(MakeROCPSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::ROCR(unsigned int period) const {
  return _cache->findAndAdd(MakeROCRSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::ROCR100(unsigned int period) const {
  return _cache->findAndAdd(MakeROCR100Series(*this, period));
}

SeriesAbstrPtr SeriesImpl::TRIX(unsigned int period) const {
  return _cache->findAndAdd(MakeTRIXSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::HTDCPeriod() const {
  return _cache->findAndAdd(MakeHTDCPeriodSeries(*this));
}

SeriesAbstrPtr SeriesImpl::HTDCPhase() const {
  return _cache->findAndAdd(MakeHTDCPhaseSeries(*this));
}

SeriesAbstrPtr SeriesImpl::HTPhasorPhase() const {
  return _cache->findAndAdd(MakeHTPhasorPhaseSeries(*this));
}

SeriesAbstrPtr SeriesImpl::HTPhasorQuadrature() const {
  return _cache->findAndAdd(MakeHTPhasorQuadratureSeries(*this));
}

SeriesAbstrPtr SeriesImpl::HTSine() const {
  return _cache->findAndAdd(MakeHTSineSeries(*this));
}

SeriesAbstrPtr SeriesImpl::HTLeadSine() const {
  return _cache->findAndAdd(MakeHTLeadSineSeries(*this));
}

SeriesAbstrPtr SeriesImpl::HTTrendMode() const {
  return _cache->findAndAdd(MakeHTTrendModeSeries(*this));
}

SeriesAbstrPtr SeriesImpl::LinearReg(unsigned int period) const {
  return _cache->findAndAdd(MakeLinearRegSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::LinearRegSlope(unsigned int period) const {
  return _cache->findAndAdd(MakeLinearRegSlopeSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::LinearRegAngle(unsigned int period) const {
  return _cache->findAndAdd(MakeLinearRegAngleSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::LinearRegIntercept(unsigned int period) const {
  return _cache->findAndAdd(MakeLinearRegInterceptSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::StdDev(unsigned int period, double nbDev) const {
  return _cache->findAndAdd(MakeStdDevSeries(*this, period, nbDev));
}

SeriesAbstrPtr SeriesImpl::Variance(unsigned int period, double nbDev) const {
  return _cache->findAndAdd(MakeVarianceSeries(*this, period, nbDev));
}

SeriesAbstrPtr SeriesImpl::TSF(unsigned int period) const {
  return _cache->findAndAdd(MakeTSFSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::CMO(unsigned int period) const {
  return _cache->findAndAdd(MakeCMOSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::Correlation(SeriesAbstrPtr series, unsigned int period) const {
  return _cache->findAndAdd(MakeCorrelationSeries(*this, *series, period));
}

SeriesAbstrPtr SeriesImpl::Beta(SeriesAbstrPtr series, unsigned int period) const {
  return _cache->findAndAdd(MakeBetaSeries(*this, *series, period));
}

SeriesAbstrPtr SeriesImpl::MOM(unsigned int period) const {
  return _cache->findAndAdd(MakeMOMSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::Momentum(unsigned int period) const {
  return MOM(period);
}

SeriesAbstrPtr SeriesImpl::MACD(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const {
  return _cache->findAndAdd(MakeMACDSeries(*this, fastPeriod, slowPeriod, signalPeriod));
}

SeriesAbstrPtr SeriesImpl::MACDSignal(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const {
  return _cache->findAndAdd(MakeMACDSignalSeries(*this, fastPeriod, slowPeriod, signalPeriod));
}

SeriesAbstrPtr SeriesImpl::MACDHist(unsigned int fastPeriod, unsigned int slowPeriod, unsigned int signalPeriod) const {
  return _cache->findAndAdd(MakeMACDHistSeries(*this, fastPeriod, slowPeriod, signalPeriod));
}

SeriesAbstrPtr SeriesImpl::MACDExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const {
  return _cache->findAndAdd(MakeMACDExtSeries(*this, fastPeriod, fastMAType,slowPeriod, slowMAType, signalPeriod, signalMAType));
}

SeriesAbstrPtr SeriesImpl::MACDSignalExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const {
  return _cache->findAndAdd(MakeMACDSignalExtSeries(*this, fastPeriod, fastMAType, slowPeriod, slowMAType, signalPeriod, signalMAType));
}

SeriesAbstrPtr SeriesImpl::MACDHistExt(unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const {
  return _cache->findAndAdd(MakeMACDHistExtSeries(*this, fastPeriod, fastMAType, slowPeriod, slowMAType, signalPeriod, signalMAType));
}

SeriesAbstrPtr SeriesImpl::MACDFix(unsigned int period) const {
  return _cache->findAndAdd(MakeMACDFixSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::MACDSignalFix(unsigned int period) const {
  return _cache->findAndAdd(MakeMACDSignalFixSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::MACDHistFix(unsigned int period) const {
  return _cache->findAndAdd(MakeMACDHistFixSeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::APO(unsigned int fastPeriod, unsigned int slowPeriod, MAType maType) const {
  return _cache->findAndAdd(MakeAPOSeries(*this, fastPeriod, slowPeriod, maType));
}

SeriesAbstrPtr SeriesImpl::T3(unsigned int period, double vFactor) const {
  return _cache->findAndAdd(MakeT3Series(*this, period, vFactor));
}

SeriesAbstrPtr SeriesImpl::TEMA(unsigned int period) const {
  return _cache->findAndAdd(MakeTEMASeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::TRIMA(unsigned int period) const {
  return _cache->findAndAdd(MakeTRIMASeries(*this, period));
}

SeriesAbstrPtr SeriesImpl::StochRSIFastK(int period, int fastKPeriod, int fastDPeriod, MAType fastDMAType) const {
  return _cache->findAndAdd(MakeStochRSIFastKSeries(*this, period, fastKPeriod, fastDPeriod, fastDMAType));
}
SeriesAbstrPtr SeriesImpl::StochRSIFastD(int period, int fastKPeriod, int fastDPeriod, MAType fastDMAType) const {
  return _cache->findAndAdd(MakeStochRSIFastDSeries(*this, period, fastKPeriod, fastDPeriod, fastDMAType));
}

SeriesAbstrPtr SeriesImpl::Sin() const {
  return _cache->findAndAdd(MakeSinSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Cos() const {
  return _cache->findAndAdd(MakeCosSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Tan() const {
  return _cache->findAndAdd(MakeTanSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Cosh() const {
  return _cache->findAndAdd(MakeCoshSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Sinh() const {
  return _cache->findAndAdd(MakeSinhSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Tanh() const {
  return _cache->findAndAdd(MakeTanhSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Acos() const {
  return _cache->findAndAdd(MakeAcosSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Asin() const {
  return _cache->findAndAdd(MakeAsinSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Atan() const {
  return _cache->findAndAdd(MakeAtanSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Ceil() const {
  return _cache->findAndAdd(MakeCeilSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Floor() const {
  return _cache->findAndAdd(MakeFloorSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Exp() const {
  return _cache->findAndAdd(MakeExpSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Sqrt() const {
  return _cache->findAndAdd(MakeSqrtSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Ln() const {
  return _cache->findAndAdd(MakeLnSeries(*this));
}
SeriesAbstrPtr SeriesImpl::Log10() const {
  return _cache->findAndAdd(MakeLog10Series(*this));
}

tradery::SeriesAbstrPtr SeriesAbstr::create() {
  return std::make_shared< EmptySeries >();
}

tradery::SeriesAbstrPtr SeriesAbstr::create(size_t size) {
  return std::make_shared< EmptySeries >(size);
}


template< typename F > Series operator_x(double value, const Series& series) {
  std::shared_ptr<SeriesImpl> si(std::make_shared< SeriesImpl >());
  const std::vector<double>& v = series.getVector();
  F op;
  for (size_t n = 0; n < series.unsyncSize(); n++) {
    si->push_back(op(value, v[n]));
  }

  si->synchronize(series.synchronizer());
  return Series(si);
}



constexpr auto plus = [](double v1, double v2)->double { return v1 + v2; };
constexpr auto minus = [](double v1, double v2)->double { return v1 - v2; };
constexpr auto mul = [](double v1, double v2)->double { return v1 * v2; };
constexpr auto division = [](double v1, double v2)->double { return v1 / v2; };

// global operators allowing using a constant as the first operand in an
// operation ( value + series etc)
CORE_API Series tradery::operator+(double value, const Series& series) { return operator_x< decltype(plus) >(value, series ); }
CORE_API Series tradery::operator-(double value, const Series& series) { return operator_x< decltype(minus) >(value, series); }
CORE_API Series tradery::operator*(double value, const Series& series) { return operator_x< decltype(mul) >(value, series); }
CORE_API Series tradery::operator/(double value, const Series& series) { return operator_x< decltype(division) >(value, series); }



CORE_API SeriesHelper::SeriesHelper(SeriesAbstrPtr s, size_t ix)
    : _ix(ix), _s(s) {
  assert(s);
}

CORE_API double SeriesHelper::operator=(double d) {
  //  COUT << _T( "value: " ) << d << std::endl;
  _s->setValue(_ix, d);
  return d;
}

CORE_API SeriesHelper::operator double() const {
  //  COUT << std::endl;
  return (*_s)[_ix];
}
