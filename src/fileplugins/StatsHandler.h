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

#include <log.h>

constexpr auto DD_STEPS = 4;
constexpr auto STAT_STEPS = 4;
constexpr auto EQ_STEPS = 2;

class StatsHandler : public SignalHandler, public CurrentPriceSource {
 private:
  StatsCalculator _totalStats;
  StatsCalculator _shortStats;
  StatsCalculator _longStats;
  StatsCalculator _buyHoldStats;

  std::shared_ptr<EquityCurve> _ec;
  std::shared_ptr<EquityCurve> _bhEc;

  std::shared_ptr<DrawdownCurve> _totalDC;
  std::shared_ptr<DrawdownCurve> _shortDC;
  std::shared_ptr<DrawdownCurve> _longDC;
  std::shared_ptr<DrawdownCurve> _bhDC;
  mutable PositionsContainer::PositionsContainerPtr _bhPos;

 public:
  StatsHandler(const Info& info)
      : SignalHandler(info), _totalStats(*this), _shortStats(*this), _longStats(*this), _buyHoldStats(*this) {
  }


  double getEqPct() const { return 16; }
  double getStatsPct() const { return 4; }
  double getPct() const { return getEqPct() + getStatsPct(); }
  double getStatsStep() const { return getStatsPct() / 4; }
  double getDateRangeStep() const { return getEqPct() / 8; }
  double getEqStep() const { return getEqPct() * 5.0 / 16.0; }
  double getDDStep() const { return getEqPct() / 16; }

  void sessionStarted() override {
    // this will make this module show span 20%
    if (sessionInfo().runtimeParams()->statsEnabled()) {
      LOG(log_info, "stats enabled");
      sessionInfo().runtimeStats()->addPct(getPct());
    }
    else if (sessionInfo().runtimeParams()->equityCurveEnabled() || sessionInfo().runtimeParams()->tradesEnabled()) {
      LOG(log_info, "only equity enabled");
      sessionInfo().runtimeStats()->addPct(getEqPct());
    }
  }

  /*
  void sessionEnded( PositionsContainer& positions )
  {
    RuntimeStats& rts = *sessionInfo().runtimeStats();
    std::cout << _T( "Calculating equity date range" ) << std::endl;
    rts.setStatus( _T( "Calculating equity date range" ) );
    EquityDateRange edr( __super::sessionInfo() );
    rts.step( getDateRangeStep() );

    std::cout << _T( "[StatsHandler::sessionEnded] 1" ) << std::endl;
    calcEqCurve( edr, positions );
    std::cout << _T( "[StatsHandler::sessionEnded] 2" ) << std::endl;
    calcStats( edr, positions );
    std::cout << _T( "[StatsHandler::sessionEnded] 3" ) << std::endl;
    calcScore();

    // calculate eq curve eventually
  }
  */

  void calcScore() {
    LOG(log_info, "Calculating scores");
    _totalStats.calculateScore(_ec->getTotalPctExposure(), _totalDC->ulcerIndex());
    _shortStats.calculateScore(_ec->getShortPctExposure(), _shortDC->ulcerIndex());
    _longStats.calculateScore(_ec->getLongPctExposure(), _longDC->ulcerIndex());
    _buyHoldStats.calculateScore(_bhEc->getTotalPctExposure(), _bhDC->ulcerIndex());
  }

  void calcStats(const DateRange& dateRange, PositionsContainer& positions) {
    LOG(log_info, "Calculating stats");
    sessionInfo().runtimeStats()->setMessage("Calculating statistics");

    double initialCapital = sessionInfo().runtimeParams()->positionSizing()->initialCapital();

    Timer timer;
    LOG(log_info, "calculating long + short stats");
    //    sessionInfo().runtimeStats()->setStatus( _T( "Calculating overall
    //    stats" ) );
    _totalStats.setDateRange(dateRange);
    _totalStats.setInitialCapital(initialCapital);
    _totalStats.calculateAll(positions);
    _totalStats.setEndingCapital(_ec->getEndingTotalEquity());
    sessionInfo().runtimeStats()->step(getStatsStep());

    LOG(log_info, "done long + short: ", timer.elapsed(), " sec");
    //    sessionInfo().runtimeStats()->setStatus( _T( "Calculating long stats"
    //    ) );
    LOG(log_info, "calculating long stats");
    _longStats.setDateRange(dateRange);
    _longStats.setInitialCapital(initialCapital);
    _longStats.calculateLong(positions);
    _longStats.setEndingCapital(_ec->getEndingLongEquity());
    sessionInfo().runtimeStats()->step(getStatsStep());

    LOG(log_info, "done long: ", timer.elapsed(), " sec");
    //    sessionInfo().runtimeStats()->setStatus( _T( "Calculating short stats"
    //    ) );
    LOG(log_info, "calculating short stats");
    _shortStats.setDateRange(dateRange);
    _shortStats.setInitialCapital(initialCapital);
    _shortStats.calculateShort(positions);
    _shortStats.setEndingCapital(_ec->getEndingShortEquity());
    sessionInfo().runtimeStats()->step(getStatsStep());

    LOG(log_info, "done short: ", timer.elapsed(), " sec");
    //    sessionInfo().runtimeStats()->setStatus( _T( "Calculating buy and hold
    //    stats" ) );
    LOG(log_info, "calculating b&h stats");
    _buyHoldStats.setDateRange(dateRange);
    _buyHoldStats.setInitialCapital(initialCapital);
    _buyHoldStats.calculateAll(getBHPositions());
    _buyHoldStats.setEndingCapital(_bhEc->getEndingTotalEquity());
    sessionInfo().runtimeStats()->step(getStatsStep());

    LOG(log_info, "done b&h: ", timer.elapsed(), " sec");
  }

  void calcEqCurve(const DateRange& dateRange, PositionsContainer& positions) {
    assert(sessionInfo().runtimeStats() != 0);
    RuntimeStats& rts = *sessionInfo().runtimeStats();

    if (_ec.get() == 0) {
      rts.setMessage("Calculating equity curve");
      rts.setStatus(RuntimeStatus::RUNNING);
      LOG(log_info, "Calculating equity curve for all positions");
      //      rts.setStatus( _T( "Calculating equity curve for generated
      //      positions" ) );
      _ec = std::make_shared< EquityCurve >(dateRange, sessionInfo(), positions, true);
      rts.step(getEqStep());
    }

    if (_bhEc.get() == 0) {
      rts.setMessage("Calculating Buy and Hold equity curve");
      rts.setStatus(RuntimeStatus::RUNNING);
      LOG(log_info, "Calculating Buy & Hold equity curve: ");
      //      rts.setStatus( _T( "Calculating equity curve for buy and hold" )
      //      );
      _bhEc = std::make_shared< EquityCurve >(dateRange, sessionInfo(), getBHPositions(), false);
      rts.step(getEqStep());
    }

    if (_totalDC.get() == 0) {
      rts.setMessage("Calculating total drawdown");
      rts.setStatus(RuntimeStatus::RUNNING);
      LOG(log_info, "Calculating total drawdown");
      //      rts.setStatus( _T( "Calculating drawdown for all positions" ) );
      _totalDC = std::make_shared< TotalDrawdownCurve >(*_ec);
      rts.step(getDDStep());
    }

    if (_shortDC.get() == 0) {
      rts.setMessage("Calculating short drawdown");
      rts.setStatus(RuntimeStatus::RUNNING);
      LOG(log_info, "Calculating short drawdown");
      //      rts.setStatus( _T( "Calculating drawdown for short positions" ) );
      _shortDC = std::make_shared< ShortDrawdownCurve >(*_ec);
      rts.step(getDDStep());
    }

    if (_longDC.get() == 0) {
      rts.setMessage("Calculating long drawdown");
      LOG(log_info, "Calculating long drawdown");
      //      rts.setStatus( _T( "Calculating drawdown for long positions" ) );
      _longDC = std::shared_ptr<DrawdownCurve>(new LongDrawdownCurve(*_ec));
      rts.step(getDDStep());
    }
    if (_bhDC.get() == 0) {
      rts.setMessage("Calculating Buy and Hold drawdown");
      rts.setStatus(RuntimeStatus::RUNNING);
      LOG(log_info, "Calculating b&h drawdown");
      //      rts.setStatus( _T( "Calculating drawdown for buy and hold" ) );
      _bhDC = std::make_shared< TotalDrawdownCurve >(*_bhEc);
      rts.step(getDDStep());
    }
  }

  PositionsContainer::PositionsContainerPtr makeBHPositions() const {
    Timer totalTimer;
    Timer dataTimer;

    double dataTime = 0;

    tradery::SymbolsIteratorPtr si = symbolsIterator();
    SymbolConstPtr symbol;

    // reset the symbols iterator
    si->reset();

    // create a positions container and positions manager
    // they will contain positions that will correspond to buy and hold for the
    // whole back-testing period
    PositionsContainer::PositionsContainerPtr pc(PositionsContainer::create());
    PositionsManagerAbstr::PositionsManagerAbstrPtr pm(PositionsManagerAbstr::create(pc, NotADateTime(), NotADateTime()));

    // the number of symbols
    unsigned int symbolsCount = 0;

    // todo: we should not be counting symbols this way, instead he symbols
    // source should return the size  todo: this will not work in the case of
    // dynamic datasources, or the results may be false
    // calculate the number of symbols
    while ((symbol = si->getNext()).get() != 0) {
      symbolsCount++; 
    }

    // reset the symbols iterator again
    si->reset();

    // use a simple b&h position sizing strategy - each position has a size =
    // total capital/ total symbols
    double capitalPerPosition = sessionInfo().runtimeParams()->positionSizing()->initialCapital() / symbolsCount;

    // go through the list of all symbols
    while ((symbol = si->getNext()).get() != 0) {
      // get data for the current symbol
      try {
        dataTimer.restart();
        BarsPtr data = sessionInfo().getData(symbol->symbol());
        Bars bars(dynamic_cast<const BarsAbstr*>(data.get()));
        dataTime += dataTimer.elapsed();

        if (bars) {
          // calculate position size
          double posSize = capitalPerPosition / bars.open(0);
          // buy at market on the first bar
          pm->buyAtMarket(bars, 0, (unsigned long)posSize, "");
          // sell at close on the last bar
          if (pc->getLastPosition()) {
            pm->sellAtClose(bars, bars.size() - 1, pc->getLastPosition(), "");
          }
        }
      }
      catch (...) {
        // todo: handle data errors - maybe catch all exceptions and ignore
        // them, we don't care why the data is not available, just don't add
        // that data to final stats
      }
      // for now just assume everything is bars
      // todo: will have to see how to handle an arbitrary datatype
    }
    return pc;
  }

  PositionsContainer& getBHPositions() const {
    if (!_bhPos) {
      _bhPos = makeBHPositions();
    }

    return *_bhPos;
  }

  virtual double get(const std::string& symbol) const {
    // make sure this is called after sessionStarted
    BarsPtr data = getData(symbol);
    // we'll assume data is bars for now
    // todo: make it work for all types of data
    Bars bars(dynamic_cast<const BarsAbstr*>(data.get()));
    if (bars) {
      return bars.close(bars.size() - 1);
    }
    else {
      throw DataNotAvailableForSymbolException();
    }
  }

  const Stats& totalStats() const { return _totalStats; }
  const Stats& shortStats() const { return _shortStats; }
  const Stats& longStats() const { return _longStats; }
  const Stats& bhStats() const { return _buyHoldStats; }

  const EquityCurve& equityCurve() const {
    assert(_ec.get() != 0);
    return *_ec;
  }
  const EquityCurve& bhEquityCurve() const {
    assert(_bhEc.get() != 0);
    return *_bhEc;
  }

  const DrawdownCurve& totalDrawdownCurve() const {
    assert(_totalDC.get() != 0);
    return *_totalDC;
  }
  const DrawdownCurve& shortDrawdownCurve() const {
    assert(_shortDC.get() != 0);
    return *_shortDC;
  }
  const DrawdownCurve& longDrawdownCurve() const {
    assert(_longDC.get() != 0);
    return *_longDC;
  }
  const DrawdownCurve& bhDrawdownCurve() const {
    assert(_bhDC.get() != 0);
    return *_bhDC;
  }
};
