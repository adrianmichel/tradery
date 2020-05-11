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
#include "positions.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void SlippageStats::onPosition(tradery::Position pos) {
  if (pos.isClosed()) {
    this->_totalCloseSlippage += pos.getCloseSlippage();
  }
  _totalOpenSlippage += pos.getEntrySlippage();
}

void CommissionStats::onPosition(tradery::Position pos) {
  if (pos.isClosed()) {
    _totalCloseCommission += pos.getCloseCommission();
  }
  _totalOpenCommission += pos.getEntryCommission();
}

void PosStats::calc(tradery::Position pos, double gain, double posPctGain) {
  _gain += gain;
  _count++;

  if (gain > 0) {
    _winningCount++;
    _totalGain += gain;
    //    COUT << _T( "total gain: " ) << _totalGain << std::endl;
  }
  else if (gain < 0) {
    _losingCount++;
    _totalLoss += gain;
    //    COUT << _T( "total loss: " ) << _totalLoss << std::endl;
  }
  else {
    _neutralCount++;
  }

  _maxGainPerPos = std::max(_maxGainPerPos, gain);
  _maxLossPerPos = std::min(_maxLossPerPos, gain);

  _maxPctGainPerPos = std::max(_maxPctGainPerPos, posPctGain);
  _maxPctLossPerPos = std::min(_maxPctLossPerPos, posPctGain);

  _totalPctGain += posPctGain;

  _totalOpenCost += pos.getEntryCost();
}

void ClosedPosStats::onPosition(tradery::Position pos) {
  if (pos.isClosed()) {
    this->_commissionStats.onPosition(pos);
    this->_slippageStats.onPosition(pos);

    double gain = pos.getGain();
    double pctGain = pos.getPctGain();

    calc(pos, gain, pctGain);
    _totalCloseCost += pos.getCloseIncome();
  }
}

void OpenPosStats::onPosition(tradery::Position pos, const CurrentPriceSource& cpr) {
  if (pos.isOpen()) {
    this->_commissionStats.onPosition(pos);
    this->_slippageStats.onPosition(pos);

    double gain = 0;

    try {
      // this is the gain considering the last price of the security for an open
      // position
      const std::string& symbol(pos.getSymbol());
      gain += pos.getGain(cpr.get(symbol));
    }
    catch (const DataNotAvailableForSymbolException&) {
      // ignore symbols for which we can't get data
      // todo: should not happen - if we could open a position, we should be
      // able to get data for it
    }
    double pctGain = gain / pos.getEntryCost() * 100;

    calc(pos, gain, pctGain);
  }
}

StatsCalculator::StatsCalculator(
    const CurrentPriceSource& currentPriceRequester)
    : _cpr(currentPriceRequester) {}

void StatsCalculator::calculate(PositionsContainer& positions, const PositionEqualPredicate& positionPredicate) {
  reset();
  positions.forEach(*this, positionPredicate);
}

void StatsCalculator::calculateAll(PositionsContainer& positions) {
  reset();
  positions.forEach(*this);
}

void StatsCalculator::calculateLong(PositionsContainer& positions) {
  calculate(positions, PositionEqualLongPredicate());
}

void StatsCalculator::calculateShort(PositionsContainer& positions) {
  calculate(positions, PositionEqualShortPredicate());
}

void StatsCalculator::onPosition(tradery::Position pos) {
  if (pos.isClosed()) {
    _closedPosStats.onPosition(pos);
  }
  else {
    _openPosStats.onPosition(pos, _cpr);
  }

  _allPosStats = _openPosStats + _closedPosStats;
}

class Gain : public PositionHandler {
 private:
  double _gain;

 public:
  Gain() : _gain(0) {}

  void onPosition(tradery::Position pos) override {
    if (pos.isClosed()) {
      _gain += pos.getGain();
    }
  }

  double operator()() const { return _gain; }
};

inline double totalGain(const PositionsContainer& pos) {
  Gain gain;
  pos.forEachConst(gain);
  return gain();
}

inline double averageGain(const PositionsContainer& pos) {
  Gain gain;

  pos.forEachConst(gain);
  return gain() / (pos.enabledCount() - pos.openPositionsCount());
}

// two sinchronized series: times and equity values
// make it a series, so we can apply all the indicators and other stats
class EquitySeries : public std::vector<double>, public PositionHandler {
 private:
  using TimeToDoubleMap = std::map<DateTime, double>;

 private:
  TimeSeries _time;
  const double _initialEquity;
  TimeToDoubleMap _m;

 public:
  EquitySeries(const PositionsContainer& pos) : _initialEquity(0) { init(pos); }

  EquitySeries(const PositionsContainer& pos, double initialEquity)
      : _initialEquity(initialEquity) {
    init(pos);
  }

  void init(const PositionsContainer& pos) {
    pos.forEachClosedConst(*this);

    double equity = _initialEquity;
    for (auto v : _m ) {
      equity += v.second;
      // set the time
      _time.push_back(v.first);
      // set total equity for the time
      push_back(equity);
    }
  }

  // called for each closed position in the pos passed to the constructor
  virtual void onPositionConst(const tradery::Position pos) {
    assert(pos.isClosed());

    // populate the map with gain/bar
    TimeToDoubleMap::iterator i = _m.find(pos.getCloseTime());
    if (i == _m.end()) {
      _m.insert(TimeToDoubleMap::value_type(pos.getCloseTime(), pos.getGain()));
    }
    else {
      i->second += pos.getGain();
    }
  }

  // the final equity value - initial equity;
  double totalProfit() const { return back() - _initialEquity; }

  // total profit / number of bars (known externally)
  // TODO: see how I can get this right
  double profitPerBar(size_t bars) const { return totalProfit() / bars; }
};
