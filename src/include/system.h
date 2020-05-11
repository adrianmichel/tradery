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
 * \brief Trading system related declarations
 */

#include "exceptions.h"
#include "core.h"
#include "macros.h"
#include "charthandler.h"
#include <collections.h>
#include <math.h>
#include <minmax.h>
#include "explicittrades.h"

using tradery::chart::Pane;

namespace tradery {

/**
 * Base class for trading systems.
 *
 * System represents a generic trading system.
 *
 * Currently it doesn't implement any functionality, but it may be useful in the
 * future, if more commonality between different types of trading systems is
 * desired.
 *
 * @see BarSystem
 * @see TickSystem
 */
class System {
 public:
  // called by user code to signal a general system error
  void systemError(const std::string& message) {
    throw GeneralSystemException(message);
  }
};

/**
 * Base class for a user defined bar type system
 *
 * The user should derive from this class to create a trading system class that
 * acts on bar type of data.
 *
 *
 * //TODO: see class Optimizer when  implemented
 *
 * The default PositionsManager instance is initialized externally by the
 * SchedulreI object that will run the system, so it will have the slippage and
 * commission passed when the system is added. Other PositionsManager objects
 * created at runtime could use different Slippage and Commission instances.
 *
 * @see Bars
 * @see Slippage
 * @see Commission
 * @see Scheduler
 */
template <class T>
class BarSystem : public Runnable,
                  public System,
                  public ClonableImpl<Runnable, T>,
                  public OpenPositionHandler,
                  public BarHandler,
                  public OrderFilter,
                  public BarIndicators,
                  public PositionsI {
 private:
  Bars _defBars;
  mutable std::vector<BarsPtr> _otherBars;
  std::string _defSymbol;
  chart::Chart* _chart;
  const tradery::ExplicitTrades* _explicitTrades;

  // TODO hide all this inside a context thing that will be instantiated in the
  // constructor in order to be able to hide implementation details and still
  // allow derivation
  Positions _defPositions;
  OpenPositionsIterator _opi;

 protected:
  // the output sink must be a static var to make it visible to member classes
  // of actual systems. otherwise, they couldn't do PrintLine
  static OutputSink* _os;

 public:
  /**
   * Creates a system with only a name but without any default values.
   * These values should be set by using the init method. The system should not
   * be run before these parameters are set. Normally, these values will be set
   * behind the scenes during the run of the system by the SchedulerI object
   *
   * @param info   Info to be associated with the datasource
   */
  BarSystem(const Info& info, const std::string& userString = "")
      : Runnable(info, userString), _defBars(0), _defPositions(0), _explicitTrades(0) {}

 public:
  /**
   * Gets the symbol associated with the default Bars object
   *
   * @return a string representing the symbol
   * @see IBars
   */
  const std::string& getSymbol() const { return _defBars.getSymbol(); }
  /**
   * called right before the run() method. It gives the system a chance to
   * perform any pre-run actions, such as not run the system on certain symbols,
   * or others.
   *
   * If it returns true, the system will continue with run, otherwise it will
   * continue with the next symbol
   *
   * @param symbol The symbol on which the system is supposed to run
   * @return return true if the system is to continue running, or false if it is
   * not to run on the current symbol
   */
  /*  virtual bool preRun( const std::string& symbol )
    {
      return true;
    }
  */
  /**
   * Pure virtual method to be implemented by the user. This will contain the
   * actual system code. It is normally called from within a SchedulerI
   * generated thread.
   */
  virtual void run() {}
  /**
   * Method called right after the run() method returns.
   *
   * Allows the system to do any post-run cleanup or other actions.
   */

  // cleanup for the current run on the current symbol
  // the default version doesn't do anything
  virtual void cleanup() {}

  virtual bool init() { return true; }

  // called by the base class
  // sets the default symbol, and calls the overload that doesn't have
  // parameters - makes it simpler for systems
  virtual bool init(const std::string& symbol) {
    // setting the "global" output sink
    if (_os == 0) _os = &outputSink();

    _defSymbol = symbol;
    return init();
  }

  // this is the bar handler for the default bars, so we can ignore the bars
  // arg, call the onBar with no Bars args
  virtual void onBar(const BarsAbstr& bar, Index index) { onBar(index); }

  virtual void onBar(Index index) {}

  virtual bool onOpenPosition(Position pos, Bars bars, Index bar) {
    return onOpenPosition(pos, bar);
  }

  virtual bool onOpenPosition(Position pos, Index bar) { return true; }

 public:
  /**
   * Sets the default parameters for the current system
   *
   * @param data      The default data collection
   * @param positions The default PositionsManager object where the created
   * positions will be added
   * @param os        Pointer to an OutputSink object which will receive the
   * output from the system
   *
   * @exception BarSystemException
   * @see Bars
   * @see PositionsManager
   * @see DataManagerI
   * @see SeriesManagerI
   */
  void init(const DataCollection* data, PositionsManagerAbstr* positions, chart::Chart* chart, const ExplicitTrades* explicitTrades) {
    assert(data != 0);
    assert(positions != 0);
    assert(chart != 0);

    try {
      _defBars = Bars(dynamic_cast<const BarsAbstr*>(data));
      if (data != 0 && !_defBars) {
        // TODO: set the right code and message
        throw BarSystemException(BAR_SYSTEM_RECEIVED_NON_BAR_DATA_ERROR, "Bar system received non-bar data");
      }
    }
    catch (std::bad_cast&) {
      throw BarSystemException(BAR_SYSTEM_RECEIVED_NON_BAR_DATA_ERROR, "Bar system received non-bar data");
    }

    _defPositions = positions;
    _opi = _defPositions.getOpenPositionsIterator();
    _chart = chart;
    _explicitTrades = explicitTrades;
  }

  void synchronize(Bars bars) {
    assert(_defBars);
    _defBars.synchronize(bars);
  }

  /**
   * Returns the default PositionsManager object as a non-const pointer
   *
   * @return A non-const pointer to the default PositionsManager object
   */
  Positions positions() {
    assert(_defPositions);
    return _defPositions;
  }

  Position getFirstOpenPosition() {
    assert(_opi);
    return _opi.getFirst();
  }

  Position getNextOpenPosition() {
    assert(_opi);
    return _opi.getNext();
  }

  bool hasOpenPositions() const {
    assert(_defPositions);
    return _defPositions.hasOpenPositions();
  }
  /**
   * Returns the default Bars object
   *
   * @return Pointer to the default Bars object
   */
  Bars bars() const {
    assert(_defBars);
    return Bars(_defBars);
  }

  Bar getBar(Index barIndex) const {
    assert(_defBars);
    return _defBars.getBar(index);
  }

  virtual void applyTimeBasedAtMarket(Bars bars, size_t barIndex, Position pos) {
    positions().applyTimeBasedAtMarket(bars, barIndex, pos);
  }
  virtual void applyTimeBasedAtClose(Bars bars, size_t barIndex, Position pos) {
    positions().applyTimeBasedAtClose(bars, barIndex, pos);
  }

  virtual void applyTimeBased(Bars bars, size_t barIndex, Position pos) {
    positions().applyTimeBased(bars, barIndex, pos);
  }

  virtual void applyStopLoss(Bars bars, size_t barIndex, Position pos) {
    positions().applyStopLoss(bars, barIndex, pos);
  }

  virtual void applyTrailing(Bars bars, size_t barIndex, Position pos) {
    positions().applyTrailing(bars, barIndex, pos);
  }

  virtual void applyBreakEvenStop(Bars bars, size_t barIndex, Position pos) {
    positions().applyBreakEvenStop(bars, barIndex, pos);
  }

  virtual void applyBreakEvenStopLong(Bars bars, size_t barIndex, Position pos) {
    positions().applyBreakEvenStopLong(bars, barIndex, pos);
  }

  virtual void applyBreakEvenStopShort(Bars bars, size_t barIndex, Position pos) {
    positions().applyBreakEvenStopShort(bars, barIndex, pos);
  }

  virtual void applyReverseBreakEvenStop(Bars bars, size_t barIndex, Position pos) {
    positions().applyReverseBreakEvenStop(bars, barIndex, pos);
  }

  virtual void applyReverseBreakEvenStopLong(Bars bars, size_t barIndex, Position pos) {
    positions().applyReverseBreakEvenStopLong(bars, barIndex, pos);
  }

  virtual void applyReverseBreakEvenStopShort(Bars bars, size_t barIndex, Position pos) {
    positions().applyReverseBreakEvenStopShort(bars, barIndex, pos);
  }

  virtual void applyProfitTarget(Bars bars, size_t barIndex, Position pos) {
    positions().applyProfitTarget(bars, barIndex, pos);
  }

  virtual void applyProfitTargetLong(Bars bars, size_t barIndex, Position pos) {
    positions().applyProfitTargetLong(bars, barIndex, pos);
  }

  virtual void applyProfitTargetShort(Bars bars, size_t barIndex, Position pos) {
    positions().applyProfitTargetShort(bars, barIndex, pos);
  }

  virtual void applyAutoStops(Bars bars, size_t barIndex) {
    positions().applyAutoStops(bars, barIndex);
  }

  PositionId buyAtMarket(Index barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) {
    assert(_defBars);
    return positions().buyAtMarket(bars(), barIndex, shares, name, applyPositionSizing);
  }

  PositionId buyAtClose(Index barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) {
    assert(_defBars);
    return positions().buyAtClose(bars(), barIndex, shares, name, applyPositionSizing);
  }

  PositionId buyAtStop(Index barIndex, double stopPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) {
    assert(_defBars);
    return positions().buyAtStop(bars(), barIndex, stopPrice, shares, name, applyPositionSizing);
  }

  PositionId buyAtLimit(Index barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) {
    assert(_defBars);
    return positions().buyAtLimit(bars(), barIndex, limitPrice, shares, name, applyPositionSizing);
  }

  bool sellAtMarket(Index barIndex, Position pos, const std::string& name) {
    assert(_defBars);
    return positions().sellAtMarket(bars(), barIndex, pos, name);
  }

  bool sellAtMarket(Index barIndex, PositionId pos, const std::string& name) {
    assert(_defBars);
    return positions().sellAtMarket(bars(), barIndex, pos, name);
  }

  bool sellAtClose(Index barIndex, Position pos, const std::string& name) {
    assert(_defBars);
    return positions().sellAtClose(bars(), barIndex, pos, name);
  }

  bool sellAtClose(Index barIndex, PositionId pos, const std::string& name) {
    assert(_defBars);
    return positions().sellAtClose(bars(), barIndex, pos, name);
  }

  bool sellAtStop(Index barIndex, Position pos, double stopPrice, const std::string& name) {
    assert(_defBars);
    return positions().sellAtStop(bars(), barIndex, pos, stopPrice, name);
  }

  bool sellAtStop(Index barIndex, PositionId pos, double stopPrice, const std::string& name) {
    assert(_defBars);
    return positions().sellAtStop(bars(), barIndex, pos, stopPrice, name);
  }

  bool sellAtLimit(Index barIndex, Position pos, double limitPrice, const std::string& name) {
    assert(_defBars);
    return positions().sellAtLimit(bars(), barIndex, pos, limitPrice, name);
  }

  bool sellAtLimit(Index barIndex, PositionId pos, double limitPrice, const std::string& name) {
    assert(_defBars);
    return positions().sellAtLimit(bars(), barIndex, pos, limitPrice, name);
  }

  PositionId shortAtMarket(Index barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) {
    assert(_defBars);
    return positions().shortAtMarket(bars(), barIndex, shares, name, applyPositionSizing);
  }

  PositionId shortAtClose(Index barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) {
    assert(_defBars);
    return positions().shortAtClose(bars(), barIndex, shares, name, applyPositionSizing);
  }

  PositionId shortAtStop(Index barIndex, double stopPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) {
    assert(_defBars);
    return positions().shortAtStop(bars(), barIndex, stopPrice, shares, name, applyPositionSizing);
  }

  PositionId shortAtLimit(Index barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) {
    assert(_defBars);
    return positions().shortAtLimit(bars(), barIndex, limitPrice, shares, name, applyPositionSizing);
  }

  bool coverAtMarket(Index barIndex, Position pos, const std::string& name) {
    assert(_defBars);
    return positions().coverAtMarket(bars(), barIndex, pos, name);
  }

  bool coverAtMarket(Index barIndex, PositionId pos, const std::string& name) {
    assert(_defBars);
    return positions().coverAtMarket(bars(), barIndex, pos, name);
  }

  bool coverAtClose(Index barIndex, Position pos, const std::string& name) {
    assert(_defBars);
    return positions().coverAtClose(bars(), barIndex, pos, name);
  }

  bool coverAtClose(Index barIndex, PositionId pos, const std::string& name) {
    assert(_defBars);
    return positions().coverAtClose(bars(), barIndex, pos, name);
  }

  bool coverAtStop(Index barIndex, Position pos, double stopPrice, const std::string& name) {
    assert(_defBars);
    return positions().coverAtStop(bars(), barIndex, pos, stopPrice, name);
  }

  bool coverAtStop(Index barIndex, PositionId pos, double stopPrice, const std::string& name) {
    assert(_defBars);
    return positions().coverAtStop(bars(), barIndex, pos, stopPrice, name);
  }

  bool coverAtLimit(Index barIndex, Position pos, double limitPrice, const std::string& name) {
    assert(_defBars);
    return positions().coverAtLimit(bars(), barIndex, pos, limitPrice, name);
  }

  bool coverAtLimit(Index barIndex, PositionId pos, double limitPrice, const std::string& name) {
    assert(_defBars);
    return positions().coverAtLimit(bars(), barIndex, pos, limitPrice, name);
  }

  void closeAllAtMarket(Index barIndex, const std::string& name) {
    assert(_defBars);
    positions().closeAllAtMarket(bars(), barIndex, name);
  }

  void closeAllShortAtMarket(Index barIndex, const std::string& name) {
    assert(_defBars);
    positions().closeAllShortAtMarket(bars(), barIndex, name);
  }

  void closeAllLongAtMarket(Index barIndex, const std::string& name) {
    assert(_defBars);
    positions().closeAllLongAtMarket(bars(), barIndex, name);
  }

  void closeAllAtClose(Index barIndex, const std::string& name) {
    assert(_defBars);
    positions().closeAllAtClose(bars(), barIndex, name);
  }

  void closeAllShortAtClose(Index barIndex, const std::string& name) {
    assert(_defBars);
    positions().closeAllShortAtClose(bars(), barIndex, name);
  }

  void closeAllLongAtClose(Index barIndex, const std::string& name) {
    assert(_defBars);
    positions().closeAllLongAtClose(bars(), barIndex, name);
  }

  void closeAllShortAtLimit(Index barIndex, double price, const std::string& name) {
    assert(_defBars);
    positions().closeAllShortAtLimit(bars(), barIndex, price, name);
  }

  void closeAllLongAtLimit(Index barIndex, double price, const std::string& name) {
    assert(_defBars);
    positions().closeAllLongAtLimit(bars(), barIndex, price, name);
  }

  void closeAllShortAtStop(Index barIndex, double price, const std::string& name) {
    assert(_defBars);
    positions().closeAllShortAtStop(bars(), barIndex, price, name);
  }

  void closeAllLongAtStop(Index barIndex, double price, const std::string& name) {
    assert(_defBars);
    positions().closeAllLongAtStop(bars(), barIndex, price, name);
  }

  void closeFirstLongAtMarketByShares(size_t shares, Index barIndex, const std::string& name) {
    assert(_defBars);
    positions().closeFirstLongAtMarketByShares(bars(), shares, barIndex, name);
  }

  void closeFirstShortAtMarketByShares(size_t shares, Index barIndex, const std::string& name) {
    assert(_defBars);
    positions().closeFirstShortAtMarketByShares(bars(), shares, barIndex, name);
  }

  PositionId buyAtMarket(Bars bars, Index barIndex, size_t shares, const std::string& name) {
    return positions().buyAtMarket(bars, barIndex, shares, name);
  }

  PositionId buyAtClose(Bars bars, Index barIndex, size_t shares, const std::string& name) {
    return positions().buyAtClose(bars, barIndex, shares, name);
  }

  PositionId buyAtStop(Bars bars, Index barIndex, double stopPrice, size_t shares, const std::string& name) {
    return positions().buyAtStop(bars, barIndex, stopPrice, shares, name);
  }

  PositionId buyAtLimit(Bars bars, Index barIndex, double limitPrice, size_t shares, const std::string& name) {
    return positions().buyAtLimit(bars, barIndex, limitPrice, shares, name);
  }

  bool sellAtMarket(Bars bars, Index barIndex, Position pos, const std::string& name) {
    return positions().sellAtMarket(bars, barIndex, pos, name);
  }

  bool sellAtMarket(Bars bars, Index barIndex, PositionId pos, const std::string& name) {
    return positions().sellAtMarket(bars, barIndex, pos, name);
  }

  bool sellAtClose(Bars bars, Index barIndex, Position pos, const std::string& name) {
    return positions().sellAtClose(bars, barIndex, pos, name);
  }

  bool sellAtClose(Bars bars, Index barIndex, PositionId pos, const std::string& name) {
    return positions().sellAtClose(bars, barIndex, pos, name);
  }

  bool sellAtStop(Bars bars, Index barIndex, Position pos, double stopPrice, const std::string& name) {
    return positions().sellAtStop(bars, barIndex, pos, stopPrice, name);
  }

  bool sellAtStop(Bars bars, Index barIndex, PositionId pos, double stopPrice, const std::string& name) {
    return positions().sellAtStop(bars, barIndex, pos, stopPrice, name);
  }

  bool sellAtLimit(Bars bars, Index barIndex, Position pos, double limitPrice, const std::string& name) {
    return positions().sellAtStop(bars, barIndex, pos, limitPrice, name);
  }

  bool sellAtLimit(Bars bars, Index barIndex, PositionId pos, double limitPrice, const std::string& name) {
    return positions().sellAtLimit(bars, barIndex, pos, limitPrice, name);
  }

  PositionId shortAtMarket(Bars bars, Index barIndex, size_t shares, const std::string& name) {
    return positions().shortAtMarket(bars, barIndex, shares, name);
  }

  PositionId shortAtClose(Bars bars, Index barIndex, size_t shares, const std::string& name) {
    return positions().shortAtClose(bars, barIndex, shares, name);
  }

  PositionId shortAtStop(Bars bars, Index barIndex, double stopPrice, size_t shares, const std::string& name) {
    return positions().shortAtStop(bars, barIndex, stopPrice, shares, name);
  }

  PositionId shortAtLimit(Bars bars, Index barIndex, double limitPrice, size_t shares, const std::string& name) {
    return positions().shortAtLimit(bars, barIndex, limitPrice, shares, name);
  }

  bool coverAtMarket(Bars bars, Index barIndex, Position pos, const std::string& name) {
    return positions().coverAtMarket(bars, barIndex, pos, name);
  }

  bool coverAtMarket(Bars bars, Index barIndex, PositionId pos, const std::string& name) {
    return positions().coverAtMarket(bars, barIndex, pos, name);
  }

  bool coverAtClose(Bars bars, Index barIndex, Position pos, const std::string& name) {
    return positions().coverAtClose(bars, barIndex, pos, name);
  }

  bool coverAtClose(Bars bars, Index barIndex, PositionId pos, const std::string& name) {
    return positions().coverAtClose(bars, barIndex, pos, name);
  }

  bool coverAtStop(Bars bars, Index barIndex, Position pos, double stopPrice, const std::string& name) {
    return positions().coverAtStop(bars, barIndex, pos, stopPrice, name);
  }

  bool coverAtStop(Bars bars, Index barIndex, PositionId pos, double stopPrice, const std::string& name) {
    return positions().coverAtStop(bars, barIndex, pos, stopPrice, name);
  }

  bool coverAtLimit(Bars bars, Index barIndex, Position pos, double limitPrice, const std::string& name) {
    return positions().coverAtLimit(bars, barIndex, pos, limitPrice, name);
  }

  bool coverAtLimit(Bars bars, Index barIndex, PositionId pos, double limitPrice, const std::string& name) {
    return positions().coverAtLimit(bars, barIndex, pos, limitPrice, name);
  }

  void closeAllAtMarket(Bars bars, Index barIndex, const std::string& name) {
    positions().closeAllAtMarket(bars, barIndex, name);
  }

  void closeAllShortAtMarket(Bars bars, Index barIndex, const std::string& name) {
    positions().closeAllShortAtMarket(bars, barIndex, name);
  }

  void closeAllLongAtMarket(Bars bars, Index barIndex, const std::string& name) {
    positions().closeAllLongAtMarket(bars, barIndex, name);
  }

  void closeAllAtClose(Bars bars, Index barIndex, const std::string& name) {
    positions().closeAllAtClose(bars, barIndex, name);
  }

  void closeAllShortAtClose(Bars bars, Index barIndex, const std::string& name) {
    positions().closeAllShortAtClose(bars, barIndex, name);
  }

  void closeAllLongAtClose(Bars bars, Index barIndex, const std::string& name) {
    positions().closeAllLongAtClose(bars, barIndex, name);
  }

  void closeAllShortAtLimit(Bars bars, Index barIndex, double price, const std::string& name) {
    positions().closeAllShortAtLimit(bars, barIndex, price, name);
  }

  void closeAllLongAtLimit(Bars bars, Index barIndex, double price, const std::string& name) {
    positions().closeAllLongAtLimit(bars, barIndex, price, name);
  }

  void closeAllShortAtStop(Bars bars, Index barIndex, double price, const std::string& name) {
    positions().closeAllShortAtStop(bars, barIndex, price, name);
  }

  void closeAllLongAtStop(Bars bars, Index barIndex, double price, const std::string& name) {
    positions().closeAllLongAtStop(bars, barIndex, price, name);
  }

  void installTimeBasedExitAtMarket(Index bars) {
    positions().installTimeBasedExitAtMarket(bars);
  }

  void installTimeBasedExitAtClose(Index bars) {
    positions().installTimeBasedExitAtClose(bars);
  }

  void installTimeBasedExit(Index bars) {
    positions().installTimeBasedExit(bars);
  }

  void installBreakEvenStop(double level) {
    positions().installBreakEvenStop(level);
  }

  void installBreakEvenStopLong(double level) {
    positions().installBreakEvenStopLong(level);
  }

  void installBreakEvenStopShort(double level) {
    positions().installBreakEvenStopShort(level);
  }

  void installReverseBreakEvenStop(double level) {
    positions().installReverseBreakEvenStop(level);
  }

  void installReverseBreakEvenStopLong(double level) {
    positions().installReverseBreakEvenStopLong(level);
  }

  void installReverseBreakEvenStopShort(double level) {
    positions().installReverseBreakEvenStopShort(level);
  }

  void installStopLoss(double level) { positions().installStopLoss(level); }

  void installProfitTarget(double level) {
    positions().installProfitTarget(level);
  }

  void installProfitTargetLong(double level) {
    positions().installProfitTargetLong(level);
  }

  void installProfitTargetShort(double level) {
    positions().installProfitTargetShort(level);
  }

  void installTrailingStop(double trigger, double lossLevel) {
    positions().installTrailingStop(trigger, lossLevel);
  }

  void applyTimeBased(Index barIndex, Position pos) {
    positions().applyTimeBased(bars(), barIndex, pos);
  }

  void applyStopLoss(Index barIndex, Position pos) {
    positions().applyStopLoss(bars(), barIndex, pos);
  }

  void applyTrailing(Index barIndex, Position pos) {
    positions().applyTrailing(bars(), barIndex, pos);
  }

  void applyBreakEvenStop(Index barIndex, Position pos) {
    positions().applyBreakEvenStop(bars(), barIndex, pos);
  }

  void applyBreakEvenStopLong(Index barIndex, Position pos) {
    positions().applyBreakEvenStopLong(bars(), barIndex, pos);
  }

  void applyBreakEvenStopShort(Index barIndex, Position pos) {
    positions().applyBreakEvenStopShort(bars(), barIndex, pos);
  }

  void applyReverseBreakEvenStop(Index barIndex, Position pos) {
    positions().applyReverseBreakEvenStop(bars(), barIndex, pos);
  }

  void applyReverseBreakEvenStopLong(Index barIndex, Position pos) {
    positions().applyReverseBreakEvenStopLong(bars(), barIndex, pos);
  }

  void applyReverseBreakEvenStopShort(Index barIndex, Position pos) {
    positions().applyReverseBreakEvenStopShort(bars(), barIndex, pos);
  }

  void applyProfitTarget(Index barIndex, Position pos) {
    positions().applyProfitTarget(bars(), barIndex, pos);
  }

  void applyProfitTargetLong(Index barIndex, Position pos) {
    positions().applyProfitTargetLong(bars(), barIndex, pos);
  }

  void applyProfitTargetShort(Index barIndex, Position pos) {
    positions().applyProfitTargetShort(bars(), barIndex, pos);
  }

  void applyAutoStops(Index barIndex) {
    positions().applyAutoStops(bars(), barIndex);
  }

  void forEachOpenPosition(Index bar) {
    positions().forEachOpenPosition(*this, bars(), bar);
  }

  void forEachBar(Index bar = 0) { forEachBar(*this, bar); }

  tradery::Position getLastPosition() { return positions().getLastPosition(); }

  tradery::Position getLastOpenPosition() {
    return positions().getLastOpenPosition();
  }

  /**
   * Gets bars data for a symbol that can be different than the current symbol
   *
   * The pointer to bars gets stored in a vector data member, to ensure that it
   * will be there for the duration of the system and it won't be removed from
   * the cache by the data manager
   *
   * When the system class gets destroyed, the data gets uncached
   */
  Bars getBars(const std::string& symbol) const {
    BarsPtr data = getData(symbol);
    if (data.get() != 0) {
      const BarsAbstr* bars = dynamic_cast<const BarsAbstr*>(data.get());
      if (bars != 0) {
        // add to the vector
        _otherBars.push_back(data);
        return Bars(bars);
      }
    }
    return Bars(symbol);
  }

  Bars bars(String symbol) const { return getBars(symbol); }

  Bars getBarsForSymbol(const std::string& symbol) const {
    return getBars(symbol);
  }

  Bars getDefaultBars() const { return bars(); }

  /**
   * Returns the series of low values of the default Bars object
   *
   * @return Pointer to the Series of lows
   * @see Series
   */
  const Series lowSeries() const {
    assert(_defBars);
    return _defBars.lowSeries();
  }
  /**
   * Returns the series of high values of the default Bars object
   *
   * @return Pointer to the Series of highs
   * @see Series
   */
  const Series highSeries() const {
    assert(_defBars);
    return _defBars.highSeries();
  }
  /**
   * Returns the series of open values of the default Bars object
   *
   * @return Pointer to the Series of open values
   * @see Series
   */
  const Series openSeries() const {
    assert(_defBars);
    return _defBars.openSeries();
  }
  /**
   * Returns the series of close values of the default Bars object
   *
   * @return Pointer to the Series of close values
   * @see Series
   */
  const Series closeSeries() const {
    assert(_defBars);
    return _defBars.closeSeries();
  }
  /**
   * Returns the series of volume values of the default Bars object
   *
   * @return Pointer to the Series of volume values
   * @see Series
   */
  const Series volumeSeries() const {
    assert(_defBars);
    return _defBars.volumeSeries();
  }
  /**
   * Returns the series of open interest values of the default Bars object
   *
   * @return Pointer to the Series of volume values
   * @see Series
   */
  const Series openInterestSeries() const {
    assert(_defBars);
    return _defBars.openInterestSeries();
  }
  /**
   * Returns the series of time values of the default Bars object
   *
   * @return const Reference to the TimeSeries object
   * @see TimeSeries
   */
  const TimeSeries& timeSeries() const {
    assert(_defBars);
    return _defBars.timeSeries();
  }
  /**
   * Gets the low value at index of the default Bars
   *
   * @param barIndex bar index of the value to be retrieved
   * @return the low at barIndex
   * @exception SeriesIndexOutOfRangeException
   *                   thrown if the index is not within the series range
   */
  double low(Index barIndex) const {
    assert(_defBars);
    return _defBars.low(barIndex);
  }
  /**
   * Gets the high value at index of the default Bars
   *
   * @param barIndex bar index of the value to be retrieved
   * @return the high at barIndex
   * @exception SeriesIndexOutOfRangeException
   *                   thrown if the index is not within the series range
   */
  double high(Index barIndex) const {
    assert(_defBars);
    return _defBars.high(barIndex);
  }
  /**
   * Gets the open value at index of the default Bars
   *
   * @param barIndex bar index of the value to be retrieved
   * @return the open at barIndex
   * @exception SeriesIndexOutOfRangeException
   *                   thrown if the index is not within the series range
   */
  double open(Index barIndex) const {
    assert(_defBars);
    return _defBars.open(barIndex);
  }
  /**
   * Gets the close value at index of the default Bars
   *
   * @param barIndex bar index of the value to be retrieved
   * @return the close at barIndex
   * @exception SeriesIndexOutOfRangeException
   *                   thrown if the index is not within the series range
   */
  double close(Index barIndex) const {
    assert(_defBars);
    return _defBars.close(barIndex);
  }
  /**
   * Gets the volume value at index of the default Bars
   *
   * @param barIndex bar index of the value to be retrieved
   * @return the volume at barIndex
   * @exception SeriesIndexOutOfRangeException
   *                   thrown if the index is not within the series range
   */
  double volume(Index barIndex) const {
    assert(_defBars);
    return _defBars.volume(barIndex);
  }
  /**
   * Gets the open interest value at index of the default Bars
   *
   * @param barIndex bar index of the value to be retrieved
   * @return the volume at barIndex
   * @exception SeriesIndexOutOfRangeException
   *                   thrown if the index is not within the series range
   */
  double openInterest(Index barIndex) const {
    assert(_defBars);
    return _defBars.openInterest(barIndex);
  }
  /**
   * Gets the date/time at index of the default Bars
   *
   * @param barIndex bar index of the value to be retrieved
   * @return the time at barIndex
   * @exception SeriesIndexOutOfRangeException
   *                   thrown if the index is not within the series range
   */
  DateTime time(Index barIndex) const {
    assert(_defBars);
    return _defBars.time(barIndex);
  }

  Date date(Index barIndex) const {
    assert(_defBars);
    return _defBars.date(barIndex);
  }

  // TODO: to make System a OpenPositionHandler?
  void forEachOpenPosition(OpenPositionHandler& handler, Bars bars, size_t barIndex) {
    assert(_defPositions);
    _defPositions.forEachOpenPosition(handler, bars, barIndex);
  }

  // TODO: to make System a BarHandler?
  void forEachBar(BarHandler& handler, Index startBar = 0) {
    assert(_defBars);
    _defBars.forEach(handler, startBar);
  }

  virtual ~BarSystem() {}

  size_t openPositionsCount() const {
    assert(_defPositions);
    return _defPositions.openPositionsCount();
  }

  Index barsCount() const {
    assert(_defBars);
    return _defBars.size();
  }

  size_t size() const {
    assert(_defBars);
    return _defBars.size();
  }

  const String& defSymbol() const { return _defSymbol; }

  // chart related APIs

  chart::Pane getDefaultPane() { return _chart->getDefaultPane(); }
  // background is not a reference in purpose, so we can use a hex to create a
  // temp Color object
  chart::Pane createPane(const std::string& name, const tradery::chart::Color background = chart::Color()) {
    return _chart->createPane(name, background);
  }
  void setChartName(const std::string& name) { _chart->setName(name); }

  ExplicitTrades& getExplicitTrades() {
    if (_explicitTrades == 0 || !_explicitTrades->hasExplicitTrades()) {
      throw SystemException(EXPLICIT_TRADES_NOT_AVAILABLE, "No explicit trades available");
    }
    else {
      return *_triggers;
    }
  }

  String getFirstSymbol() const {
    assert(symbolsIterator() != 0);
    SymbolConstPtr symbol = symbolsIterator()->getFirst();
    if (symbol) {
      return *symbol;
    }
    else {
      return "";
    }
  }

  String getNextSymbol() const {
    assert(symbolsIterator() != 0);
    SymbolConstPtr symbol = symbolsIterator()->getNext();
    if (symbol) {
      return *symbol;
    }
    else {
      return "";
    }
  }

  void resetSymbols() const {
    assert(symbolsIterator() != 0);
    symbolsIterator()->reset();
  }

  // auto trigger methods: (
  void autoApplyExplicitTrades(size_t barIndex) {
    const DateTime time(_defBars.time(barIndex));

    if (_explicitTrades != 0) {
      _explicitTrades->process(_defSymbol, time, barIndex, _defPositions, _defBars);
    }
  }

  size_t positionCount() const override { return _defPositions.positionCount(); }

  Position getPosition(PositionId id) {
    Position pos(_defPositions.getPosition(id));

    if (pos)
      return pos;
    else
      throw PositionIdNotFoundException(id);
  }

  double getInitialCapital() const {
    assert(sessionInfo().runtimeParams() != 0);
    assert(sessionInfo().runtimeParams()->positionSizing() != 0);

    return sessionInfo().runtimeParams()->positionSizing()->initialCapital();
  }

  // bar indicators applied to the default bars

  const Series TrueRange() const { return bars().TrueRange(); }

  const Series AccumDist() const { return bars().AccumDist(); }

  const Series ADX(unsigned int period) const { return bars().ADX(period); }

  const Series ADXR(unsigned int period) const { return bars().ADXR(period); }

  const Series MinusDI(unsigned int period) const {
    return bars().MinusDI(period);
  }

  const Series MinusDM(unsigned int period) const {
    return bars().MinusDM(period);
  }

  const Series AvgPrice() const { return bars().AvgPrice(); }

  const Series MedPrice() const { return bars().MedPrice(); }

  const Series TypPrice() const { return bars().TypPrice(); }

  const Series WclPrice() const { return bars().WclPrice(); }

  const Series CCI(unsigned int period) const { return bars().CCI(period); }

  const Series DX(unsigned int period) const { return bars().DX(period); }

  const Series ChaikinAD() const { return bars().ChaikinAD(); }

  const Series BOP() const { return bars().BOP(); }

  const Series ChaikinADOscillator(unsigned int fastPeriod, unsigned int slowPeriod) const {
    return bars().ChaikinADOscillator(fastPeriod, slowPeriod);
  }

  const Series OBV(const Series& series) const { return bars().OBV(series); }

  const Series MidPrice(unsigned int period) const {
    return bars().MidPrice(period);
  }

  const Series SAR(double acceleration, double maximum) const {
    return bars().SAR(acceleration, maximum);
  }

  const Series MFI(unsigned int period) const { return bars().MFI(period); }

  const Series ATR(unsigned int period) const { return bars().ATR(period); }

  const Series TR() const { return bars().TR(); }

  const Series PlusDI(unsigned int period) const {
    return bars().PlusDI(period);
  }

  const Series PlusDM(unsigned int period) const {
    return bars().PlusDM(period);
  }

  const Series WillR(unsigned int period) const { return bars().WillR(period); }

  const Series NATR(unsigned int period) const { return bars().NATR(period); }

  const Series StochSlowK(int fastKPeriod, int slowKPeriod, MAType slowKMAType, int slowDPeriod, MAType slowDMAType) const {
    return bars().StochSlowK(fastKPeriod, slowKPeriod, slowKMAType, slowDPeriod, slowDMAType);
  }

  const Series StochSlowD(int fastKPeriod, int slowKPeriod, MAType slowKMAType, int slowDPeriod, MAType slowDMAType) const {
    return bars().StochSlowD(fastKPeriod, slowKPeriod, slowKMAType, slowDPeriod, slowDMAType);
  }

  const Series StochFastK(int fastKPeriod, int fastDPeriod, MAType fastDMAType) const {
    return bars().StochFastK(fastKPeriod, fastDPeriod, fastDMAType);
  }

  const Series StochFastD(int fastKPeriod, int fastDPeriod, MAType fastDMAType) const {
    return bars().StochFastD(fastKPeriod, fastDPeriod, fastDMAType);
  }

  /**\name Candle patterns
   * @{
   */
  const Series Cdl3BlackCrows() const { return bars().Cdl3BlackCrows(); }

  const Series CdlAbandonedBaby(double penetration) const {
    return bars().CdlAbandonedBaby(penetration);
  }

  const Series CdlDojiStar() const { return bars().CdlDojiStar(); }

  const Series CdlEngulfing() const { return bars().CdlEngulfing(); }

  const Series CdlEveningDojiStar(double penetration) const {
    return bars().CdlEveningDojiStar(penetration);
  }

  const Series CdlEveningStar(double penetration) const {
    return bars().CdlEveningStar(penetration);
  }

  const Series CdlHammer() const { return bars().CdlHammer(); }

  const Series CdlHangingMan() const { return bars().CdlHangingMan(); }

  const Series CdlHarami() const { return bars().CdlHarami(); }

  const Series CdlHaramiCross() const { return bars().CdlHaramiCross(); }

  const Series CdlHighWave() const { return bars().CdlHighWave(); }

  const Series CdlIdentical3Crows() const {
    return bars().CdlIdentical3Crows();
  }

  const Series CdlInvertedHammer() const { return bars().CdlInvertedHammer(); }

  const Series CdlLongLine() const { return bars().CdlLongLine(); }

  const Series CdlMorningDojiStar(double penetration) const {
    return bars().CdlMorningDojiStar(penetration);
  }

  const Series CdlMorningStar(double penetration) const {
    return bars().CdlMorningStar(penetration);
  }

  const Series CdlShootingStar() const { return bars().CdlShootingStar(); }

  const Series CdlShortLine() const { return bars().CdlShortLine(); }

  const Series CdlSpinningTop() const { return bars().CdlSpinningTop(); }

  const Series CdlTristar() const { return bars().CdlTristar(); }

  const Series CdlUpsideGap2Crows() const {
    return bars().CdlUpsideGap2Crows();
  }

  // b 0.1.3
  const Series Cdl2Crows() const { return bars().Cdl2Crows(); }

  const Series Cdl3Inside() const { return bars().Cdl3Inside(); }

  const Series Cdl3LineStrike() const { return bars().Cdl3LineStrike(); }

  const Series Cdl3WhiteSoldiers() const { return bars().Cdl3WhiteSoldiers(); }

  const Series Cdl3Outside() const { return bars().Cdl3Outside(); }

  const Series CdlInNeck() const { return bars().CdlInNeck(); }

  const Series CdlOnNeck() const { return bars().CdlOnNeck(); }

  const Series CdlPiercing() const { return bars().CdlPiercing(); }

  const Series CdlStalled() const { return bars().CdlStalled(); }

  const Series CdlThrusting() const { return bars().CdlThrusting(); }

  const Series CdlAdvanceBlock(double penetration) const {
    return bars().CdlAdvanceBlock(penetration);
  }

  // b 0.1.4
  const Series Cdl3StarsInSouth() const { return bars().Cdl3StarsInSouth(); }

  const Series CdlBeltHold() const { return bars().CdlBeltHold(); }

  const Series CdlBreakaway() const { return bars().CdlBreakaway(); }

  const Series CdlClosingMarubuzu() const {
    return bars().CdlClosingMarubuzu();
  }

  const Series CdlConcealingBabySwallow() const {
    return bars().CdlConcealingBabySwallow();
  }

  const Series CdlCounterattack() const { return bars().CdlCounterattack(); }

  const Series CdlDoji() const { return bars().CdlDoji(); }

  const Series CdlDragonFlyDoji() const { return bars().CdlDragonFlyDoji(); }

  const Series CdlGapSideBySideWhite() const {
    return bars().CdlGapSideBySideWhite();
  }

  const Series CdlGravestoneDoji() const { return bars().CdlGravestoneDoji(); }

  const Series CdlHomingPigeon() const { return bars().CdlHomingPigeon(); }

  const Series CdlKicking() const { return bars().CdlKicking(); }

  const Series CdlKickingByLength() const {
    return bars().CdlKickingByLength();
  }

  const Series CdlLadderBottom() const { return bars().CdlLadderBottom(); }

  const Series CdlLongLeggedDoji() const { return bars().CdlLongLeggedDoji(); }

  const Series CdlMarubozu() const { return bars().CdlMarubozu(); }

  const Series CdlMatchingLow() const { return bars().CdlMatchingLow(); }

  const Series CdlRickshawMan() const { return bars().CdlRickshawMan(); }

  const Series CdlRiseFall3Methods() const {
    return bars().CdlRiseFall3Methods();
  }

  const Series CdlSeparatingLines() const {
    return bars().CdlSeparatingLines();
  }

  const Series CdlStickSandwich() const { return bars().CdlStickSandwich(); }

  const Series CdlTakuri() const { return bars().CdlTakuri(); }

  const Series CdlTasukiGap() const { return bars().CdlTasukiGap(); }

  const Series CdlUnique3River() const { return bars().CdlUnique3River(); }

  const Series CdlXSideGap3Methods() const {
    return bars().CdlXSideGap3Methods();
  }

  const Series CdlHikkake() const { return bars().CdlHikkake(); }

  const Series CdlMatHold(double penetration) const {
    return bars().CdlMatHold(penetration);
  }
};

template <class T>
OutputSink* BarSystem<T>::_os = 0;

/**
 * A base class for trading systems that run on tick data.
 *
 * @see Runnable
 * @see BarSystem
 */
class CORE_API TickSystem : public Runnable, public System {
 private:
  const Ticks* _defTicks;

  PositionsManagerAbstr* _defPositions;

 public:
  /**
   * Constructor that takes the name of the tick system as argument
   *
   * @param info the info for this system
   */
  TickSystem(const Info& info)
      : Runnable(info, ""), _defTicks(0), _defPositions(0) {}

  /**
   * Method called before the sytem is run
   *
   * Sets the sytem default parameters: data collection ( the ticks) and a
   * position manager, which will store positions created during the system run
   *
   * @param data      The collection of ticks on which to run the system
   * @param positions The collection of positions to which to add the positions
   * that are created during the system run
   * @exception TickSystemException
   */
  void init(const DataCollection* data,
            PositionsManagerAbstr* positions) {
    assert(data != 0);
    assert(positions != 0);

    _defTicks = dynamic_cast<const Ticks*>(data);
    if (data != 0 && _defTicks == 0) {
      // TODO: set the right code and message
      throw TickSystemException(TICK_SYSTEM_RECEIVED_NON_TICK_DATA_ERROR, "Tick system received non-tick data");
    }

    _defPositions = positions;
  }

  /**
   * Gets the symbol associated with the default Bars object
   *
   * @return a string representing the symbol
   * @see IBars
   */
  const std::string& getSymbol() const { return _defTicks->getSymbol(); }

  const Ticks* defTicks() const { return _defTicks; }
};

}  // namespace tradery
