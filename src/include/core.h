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

#include <memory>
#include "log.h"
/**
 * \mainpage The TradingApp Platform API
 *
 * \section intro Introduction
 *
 * All names are in the simlib namespace
 *
 * All these declarations and defintions are found in several C++ header (.h)
 * files:
 * - datasource.h - all data source and symbols list related
 * declarations/definitions. Also has definitions for data types such as bars,
 * ticks and the associated indicators
 * - exceptions.h - all exceptions
 * - misc.h - various declarations, such as string types, smart pointers
 * classes, time/date and others
 * - miscwin.h - various declarations that are windows specific, such as
 * registry classes, plugin registry persistence
 * - optimizer.h - an optimizer class
 * - plugin.h - plugin extensibility related classes
 * - pluginhelper.h - plugin helper classes for registry configuration
 * persisence
 * - series.h - the Series class, including all the series based indicators
 * - core.h - functionality necessary to write and run trading systems
 * - system.h - the System class, the base class for most trading systems
 * - tokenizer.h - a string tokenizer class
 *
 * For an example of how to use all these to write a trading application please
 * refer to the developer guide and various sample projects projects
 *
 */

#pragma warning(disable : 4290)

#ifdef SIMLIB_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

#include <iomanip>
#include "datasource.h"
#include "macros.h"

/**
 * namespace for all SimLib names
 */
namespace tradery {

namespace chart {
class Chart;
}

/** @file
 *  \brief Contains all the SimLib exported declarations
 */
/**
 * Aabstract class - base for all concrete Slippage classes
 *
 * Concrete classes derived from Slippage will implement different
 * algorithms for calculating the slippage value for a given
 * trade.
 *
 * As Slippage is usually implemented as a plugin, it is derived from Info,
 * therefore a Slippage has a name, a description and a unique Id.
 *
 * @see Info
 * @see Plugin
 */
class Slippage : public PluginConfiguration {
 public:
  /**
   * Constructor - takes a reference to an Info object containing
   *
   * @param info   The Slippage info
   */
  Slippage(const Info& info) : PluginConfiguration(info) {}
  virtual ~Slippage() {}

  /**
   * Returns the calculated slippage value.
   *
   * Pure virtual method to be overriden in derived classes
   *
   * @param shares Number of shares
   * @param volume Volume
   * @param price  Price
   *
   * @return The calculated slippage value
   */
  virtual double getValue(unsigned long shares, unsigned long volume, double price) const = 0;

  virtual void reset() {}
};

/**
 * A smart pointer to a Slippage object
 */
using SlippagePtr = std::shared_ptr<Slippage>;

/**
 * Aabstract class - base for all concrete Commission classes
 *
 * Concrete classes derived from Commission will implement different
 * algorithms for calculating the commission value for a each trade.
 *
 * As Commission is usually implemented as a plugin, it is derived from Info,
 * therefore a Commission has a name, a description and a unique Id.
 */
class Commission : public PluginConfiguration {
 public:
  /**
   * Constructor - takes a reference to an Info object containing
   *
   * @param info   The Commission info
   */
  Commission(const Info& info) : PluginConfiguration(info) {}

  virtual ~Commission() {}
  /**
   * Returns the calculated commission value
   *
   * Pure virtual method to be overriden in derived classes
   *
   * @param shares Number of shares
   * @param price  Price
   * @return The calculated commission value
   */
  virtual double getValue(unsigned long shares, double price) const = 0;
  /**
   * Not used
   */
  virtual void reset() {}
};

/**
 * A smart pointer to a Commission object
 */
using CommissionPtr = std::shared_ptr<Commission>;

/**
 * Base class for per position user data
 *
 * Pointes to objects of classes derived from PositionData can be associated
 * with any position and they can contain any information.
 *
 * Note: it is user's responsibility to delete these objects, they are not
 * deleted by the platform.
 *
 * @see Position
 */
class PositionUserData {
 public:
  virtual ~PositionUserData() {}
};

// a unique id, identifying a position
// the ids are unique per session usually
using PositionId = unsigned __int64;

enum OrderType {
  market_order,
  limit_order,
  stop_order,
  close_order,
  price_order
};

/**
 * Abstract class - base for position class
 *
 * A Position object represents a simulated trading position.
 *
 * A Position object stores all the information usually associated with a
 * position: shares, type (long/short), status (open/closed), open time, close
 * time etc.
 *
 * In addition to these, a position also stores information that is specific to
 * the back-testing process, such as the simulated slippage calculated for each
 * side (open/close) of a position, as well as information about the various
 * stops that may have been usef for the position during back-testing, such as
 * break even, trailing stop etc.
 *
 * A position also stores the names associated with the entry/exit methods
 * (buyAt.. sellAt.. etc).
 *
 * Methods that require information specific to closed positions will throw an
 * exception if used on an open position.
 *
 * Position objects are never created directly by the user, they are created
 * internally by the framework during the back-testing process.
 *
 * Positions are stored in PositionsContainer objects
 *
 * The user can however associate user defined data with each position by
 * passing a pointer to a PositionUserData derived object.
 *
 * Note: the pointer to PositionUserData derived object is not deleted by the
 * Position destructor - it is user's responsbility to delete it when it is no
 * longer necessary
 *
 * @see PositionUserData
 * @see PositionsCollection
 */
class CORE_API PositionAbstr {
 public:
  // returns true if t1 < t2. if t1 and t2 are limit or stop, it returns true
  static bool orderTypeLower(OrderType t1, OrderType t2) {
    if (t1 == market_order || t2 == close_order) {
      return true;
    }
    else if (t1 == close_order || t2 == market_order) {
      return false;
    }
    else {
      return true;
    }
  }

  virtual ~PositionAbstr() {}

  virtual bool applyPositionSizing() const = 0;
  virtual const std::string& getUserString() const = 0;

  virtual void setShares(size_t shares) = 0;
  virtual void disable() = 0;

  /**
   * Returns the unique id of the current position
   *
   * A valid id can't be 0
   *
   * @return
   */
  virtual PositionId getId() const = 0;
  virtual bool isDisabled() const = 0;
  virtual bool isEnabled() const = 0;
  /**
   * Indicates if trailing stop is active for this position
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO:  clarify what trailing stop is and
   * how it relates to similar methods in PositionsManager </FONT> \endhtmlonly
   * -->
   *
   * @return
   * @see activateTrailingStop
   * @see getTrailingStopLevel
   */
  virtual bool isTrailingStopActive() const = 0;
  /**
   * Indicates if break even stop is active for this position
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: clarify what break even stop is and
   * how it relates to similar methods in PositionsManager </FONT> \endhtmlonly
   * -->
   *
   * @return true if break even stop is active
   * @see activateBreakEvenStop
   */
  virtual bool isBreakEvenStopActive() const = 0;
  virtual bool isBreakEvenStopLongActive() const = 0;
  virtual bool isBreakEvenStopShortActive() const = 0;
  /**
   * Indicates if reverse break even stop is active for this position
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: clarify what break even stop is and
   * how it relates to similar methods in PositionsManager </FONT> \endhtmlonly
   * -->
   *
   * @return true if reverse break even stop is active
   * @see activateReverseBreakEvenStop
   */
  virtual bool isReverseBreakEvenStopActive() const = 0;
  virtual bool isReverseBreakEvenStopLongActive() const = 0;
  virtual bool isReverseBreakEvenStopShortActive() const = 0;
  /**
   * Returns the trailing stop level set with activateTrailingStop
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO:  clarify what trailing stop is and
   * how it relates to similar methods in PositionsManager </FONT> \endhtmlonly
   * -->
   *
   * @return trailing stop level
   * @see activateTrailingStop
   * @see isTrailingStopActive
   */
  virtual double getTrailingStopLevel() const = 0;
  /**
   * Activates trailing stop for the current position
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO:  clarify what trailing stop is and
   * how it relates to similar methods in PositionsManager </FONT> \endhtmlonly
   * -->
   *
   * @param level  Trailing stop level
   */
  virtual void activateTrailingStop(double level) = 0;
  /**
   * Activates break even stop on the position
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: clarify what break even stop is and
   * how it relates to similar methods in PositionsManager </FONT> \endhtmlonly
   * -->
   */
  virtual void activateBreakEvenStop() = 0;
  /**
   * Activates reverse break even stop on the position
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: clarify what reverse break even stop
   * is and how it relates to similar methods in PositionsManager </FONT>
   * \endhtmlonly -->
   */
  virtual void activateReverseBreakEvenStop() = 0;
  /**
   * Returns the number of shares in this position
   *
   * @return Number of shares
   */
  virtual size_t getShares() const = 0;
  /**
   * Associates a pointer to a PositionUserData derived object
   * with the position.
   *
   * This pointer is owned by the calling code, so is user's responsibility to
   * delete it
   *
   * @param data   Pointer to a PositionUserData derived object
   */
  virtual void setPositionUserData(const PositionUserData* data) = 0;
  /**
   * Returnes the pointer to a PositionUserData derived object associated
   * with this position
   *
   * @return Pointer to a PositionUserData derived object
   */
  virtual const PositionUserData* getPositionUserData() const = 0;
  /**
   * Indicates whether the current position is long
   *
   * @return true if long position, false otherwise
   */
  virtual bool isLong() const = 0;
  /**
   * Indicates whether the current position is short
   *
   * @return true if short position, false otherwise
   */
  virtual bool isShort() const = 0;

  /**
   * Get the symbol associated with this position
   *
   * @return The symbol
   */
  virtual const std::string& getSymbol() const = 0;
  /**
   * Indicates whether the current position is open
   *
   * @return true if position is open, false otherwise
   */
  virtual const bool isOpen() const = 0;
  /**
   * Indicates whether the current position is closed
   *
   * @return true if position closed, false otherwise
   */
  virtual const bool isClosed() const = 0;
  virtual OrderType getEntryOrderType() const = 0;
  virtual OrderType getExitOrderType() const = 0;
  /**
   * Returns the bar index at which the position was opened
   *
   * @return Bar index on which the position was opened
   */
  virtual size_t getEntryBar() const = 0;
  /**
   * Returns the bar index at which the position was closed
   *
   * Can be applied only to open positions, or an exception will be thrown
   *
   * @return The bar index on which the position was closed
   * @exception PositionCloseOperationOnOpenPositionException
   *                   Thrown if the position is still open
   */
  virtual size_t getCloseBar() const = 0;
  /**
   * Returns the gain realized on the current position
   *
   * Can be applied only to closed positions. If applied to a still open
   * position, an exception will be thrown
   *
   * Gain is calculated by subtracting the entry cost from the exit cost (for
   * long positions) or the exit cost from the entry cost (for short positions).
   * The cost for both entry and exit includes the commission, if any
   *
   * @return The gain realized
   * @exception PositionCloseOperationOnOpenPositionException
   *                   Thrown if the position is still open
   */
  virtual double getGain() const = 0;
  virtual double getPctGain() const = 0;
  /**
   * Gain if the exit price were to be "price"
   *
   * doesn't add an exit commission - the gain is "virtual"
   */
  virtual double getGain(double price) const = 0;
  virtual double getPctGain(double price) const = 0;

  virtual double getEntryCost(size_t shares) const = 0;
  virtual double getEntryCost() const = 0;
  virtual double getCloseIncome() const = 0;
  /**
   * Returns the closing price for the position
   *
   * Can be applied only to open positions, or an exception will be thrown
   *
   * @return Closing price
   * @exception PositionCloseOperationOnOpenPositionException
   *                   Thrown if the position is still open
   */
  virtual double getClosePrice() const = 0;
  /**
   * Returns the entry price for the position
   *
   * @return entry price
   */
  virtual double getEntryPrice() const = 0;
  /**
   * Returns the entry time and data for the current position
   *
   * @return entry time and data for the current position
   */
  virtual const DateTime getEntryTime() const = 0;
  /**
   * Returns the closing time and date for the current position, if it is a
   * closed position.
   *
   * Can be applied only to open positions, or an exception will be thrown
   *
   * @return The closing time and date for the current position
   * @exception PositionCloseOperationOnOpenPositionException
   *                   Thrown if the position is still open
   */
  virtual const DateTime getCloseTime() const = 0;
  /**
   * Returns slippage value calculated when opening this position during
   * back-testing
   *
   * @return
   */
  virtual double getEntrySlippage() const = 0;
  /**
   * Returns the commission value calculated when opening this position during
   * back-testing
   *
   * @return
   */
  virtual double getEntryCommission() const = 0;
  /**
   * Returns slippage value calculated when closing this position during
   * back-testing
   *
   * Can be applied only to open positions, or an exception will be thrown
   *
   * @return
   * @exception PositionCloseOperationOnOpenPositionException
   */
  virtual double getCloseSlippage() const = 0;
  /**
   * Returns commission value calculated when opening this position during
   * back-testing
   *
   * Can be applied only to open positions, or an exception will be thrown
   *
   * @return
   * @exception PositionCloseOperationOnOpenPositionException
   */
  virtual double getCloseCommission() const = 0;
  /**
   * Returns the name used in the position opening method
   *
   * Methods such as buyAt..., shortAt.. etc have a strin parameter, which is
   * used to describe the order. This string is also stored in the position and
   * can be retrieved by calling this method
   *
   * @return
   */
  virtual const std::string& getEntryName() const = 0;
  /**
   * Returns the name used in the position closing method
   *
   * Methods such as buyAt..., shortAt.. etc have a strin parameter, which is
   * used to describe the order. This string is also stored in the position and
   * can be retrieved by calling this method
   *
   * Can be applied only to open positions, or an exception will be thrown
   *
   * @return
   * @exception PositionCloseOperationOnOpenPositionException
   */
  virtual const std::string& getCloseName() const = 0;
};

using PositionAbstrPtr = std::shared_ptr<tradery::PositionAbstr>;

class Position : public PositionAbstr {
 private:
  PositionAbstrPtr _pos;

 private:
  bool isValid() const { return (bool)_pos; }
  bool validate() const {
    if (isValid()) {
      return true;
    }
    else {
      throw InvalidPositionException();
    }
  }

 public:
  Position(PositionAbstrPtr pos) : _pos(pos) { assert(pos); }

  Position() : _pos(0) {}

  virtual bool applyPositionSizing() const {
    validate();
    return _pos->applyPositionSizing();
  }
  const std::string& getUserString() const {
    validate();
    return _pos->getUserString();
  }

  PositionAbstrPtr getPos() { return _pos; }

  operator bool() const { return isValid(); }
  virtual void disable() {
    validate();
    _pos->disable();
  }

  virtual void setShares(size_t shares) {
    validate();
    _pos->setShares(shares);
  }
  virtual PositionId getId() const {
    validate();
    return _pos->getId();
  }
  virtual bool isDisabled() const {
    validate();
    return _pos->isDisabled();
  }
  virtual bool isEnabled() const {
    validate();
    return _pos->isEnabled();
  }
  virtual bool isTrailingStopActive() const {
    validate();
    return _pos->isTrailingStopActive();
  }
  virtual bool isBreakEvenStopActive() const {
    validate();
    return _pos->isBreakEvenStopActive();
  }
  virtual bool isBreakEvenStopLongActive() const {
    validate();
    return _pos->isBreakEvenStopLongActive();
  }
  virtual bool isBreakEvenStopShortActive() const {
    validate();
    return _pos->isBreakEvenStopShortActive();
  }
  virtual bool isReverseBreakEvenStopActive() const {
    validate();
    return _pos->isReverseBreakEvenStopActive();
  }
  virtual bool isReverseBreakEvenStopLongActive() const {
    validate();
    return _pos->isReverseBreakEvenStopLongActive();
  }
  virtual bool isReverseBreakEvenStopShortActive() const {
    validate();
    return _pos->isReverseBreakEvenStopShortActive();
  }
  virtual double getTrailingStopLevel() const {
    validate();
    return _pos->getTrailingStopLevel();
  }
  virtual void activateTrailingStop(double level) {
    validate();
    _pos->activateTrailingStop(level);
  }
  virtual void activateBreakEvenStop() {
    validate();
    _pos->activateBreakEvenStop();
  }
  virtual void activateReverseBreakEvenStop() {
    validate();
    _pos->activateReverseBreakEvenStop();
  }
  virtual size_t getShares() const {
    validate();
    return _pos->getShares();
  }
  virtual void setPositionUserData(const PositionUserData* data) {
    validate();
    _pos->setPositionUserData(data);
  }
  virtual const PositionUserData* getPositionUserData() const {
    validate();
    return _pos->getPositionUserData();
  }
  virtual bool isLong() const {
    validate();
    return _pos->isLong();
  }
  virtual bool isShort() const {
    validate();
    return _pos->isShort();
  }
  virtual const std::string& getSymbol() const {
    validate();
    return _pos->getSymbol();
  }
  virtual const bool isOpen() const {
    validate();
    return _pos->isOpen();
  }
  virtual const bool isClosed() const {
    validate();
    return _pos->isClosed();
  }
  virtual OrderType getEntryOrderType() const {
    validate();
    return _pos->getEntryOrderType();
  }
  virtual OrderType getExitOrderType() const {
    validate();
    return _pos->getExitOrderType();
  }
  virtual OrderType getCloseOrderType() const {
    validate();
    return _pos->getExitOrderType();
  }
  virtual size_t getEntryBar() const {
    validate();
    return _pos->getEntryBar();
  }
  virtual size_t getCloseBar() const {
    validate();
    return _pos->getCloseBar();
  }
  virtual double getGain() const {
    validate();
    return _pos->getGain();
  }
  virtual double getPctGain() const {
    validate();
    return _pos->getPctGain();
  }
  virtual double getGain(double price) const {
    validate();
    return _pos->getGain(price);
  }
  double getGain(double price1, double price2) const {
    validate();
    double gain = (price2 - price1) * getShares();
    return isLong() ? gain : -gain;
  }
  // duration in bars
  size_t getDuration() const {
    validate();
    return getCloseBar() - getEntryBar();
  }
  virtual double getPctGain(double price) const {
    validate();
    return _pos->getPctGain(price);
  }
  virtual double getEntryCost() const {
    validate();
    return _pos->getEntryCost();
  }
  virtual double getEntryCost(size_t shares) const {
    validate();
    return _pos->getEntryCost(shares);
  }
  virtual double getCloseIncome() const {
    validate();
    return _pos->getCloseIncome();
  }
  virtual double getCloseIncome(double price) const {
    validate();
    double income = getCloseIncome() - getShares() * price;
    return isLong() ? income : -income;
  }
  virtual double getClosePrice() const {
    validate();
    return _pos->getClosePrice();
  }
  virtual double getEntryPrice() const {
    validate();
    return _pos->getEntryPrice();
  }
  virtual const DateTime getEntryTime() const {
    validate();
    return _pos->getEntryTime();
  }
  virtual const DateTime getCloseTime() const {
    validate();
    return _pos->getCloseTime();
  }
  const Date getEntryDate() const {
    validate();
    return getEntryTime().date();
  }
  const Date getCloseDate() const {
    validate();
    return getCloseTime().date();
  }
  virtual double getEntrySlippage() const {
    validate();
    return _pos->getEntrySlippage();
  }
  virtual double getEntryCommission() const {
    validate();
    return _pos->getEntryCommission();
  }
  virtual double getCloseSlippage() const {
    validate();
    return _pos->getCloseSlippage();
  }
  virtual double getCloseCommission() const {
    validate();
    return _pos->getCloseCommission();
  }
  virtual const std::string& getEntryName() const {
    validate();
    return _pos->getEntryName();
  }
  virtual const std::string& getCloseName() const {
    validate();
    return _pos->getCloseName();
  }
};

/**
 * Base class for order filters
 *
 * <!-- \htmlonly <FONT COLOR="RED">TODO: add more parameters to the filters
 * (detail about the order, price, size etc)</FONT> </FONT> \endhtmlonly -->
 *
 * An order filter is a class that receives notifications from the framework
 * when one of the order methods such as buyAtMarket etc are called
 *
 * Methods of an order filter object are never called directly by the user code.
 * They are called every time a position entry/exit method is
 * called on a PositionsManager object. They allow a user to decide, in case of
 * entry orders (buy/short) whether to actually allow the order to continue, and
 * if yes, how many shares should be bought/shorted; in case of exit order
 * (sell/cover) they enable a user to allow/disallow the order.
 *
 * Normally a user will derive a new filter class from this base class and
 * override the virtual methods that are of interests for its specific code. An
 * object of the new class will be registered with a PositionsManager object in
 * order to apply the filters.
 *
 * This class could be used for position sizing for example, or for any other
 * type of filtering that makes logical sense to be separated from the system
 * itself
 *
 * The purpose of this filter class is to allow a separation between the trading
 * system logic and various filters based on user defined criteria which
 *
 * The default filter leaves all orders unchanged.
 *
 * @see PositionManager
 */
class CORE_API OrderFilter {
 public:
  virtual ~OrderFilter() {}
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful. It could apply a position sizing algorithm and
   * change the number of shares/contracts, or it could block the trade
   * altogether
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param shares Initial number of shares
   *
   * @return 0 to block the trade, or the number of shares if >0
   * @see PositionsManager
   */
  virtual unsigned int onBuyAtMarket(size_t bar, unsigned int shares) const {
    return shares;
  }
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful. It could apply a position sizing algorithm and
   * change the number of shares/contracts, or it could block the trade
   * altogether
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param shares Initial number of shares
   *
   * @return 0 to block the trade, or the number of shares if >0
   * @see PositionsManager
   */
  virtual unsigned int onBuyAtClose(size_t bar, unsigned int shares) const {
    return shares;
  };
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful. It could apply a position sizing algorithm and
   * change the number of shares/contracts, or it could block the trade
   * altogether
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param shares Initial number of shares
   * @param price  The stop price
   *
   * @return 0 to block the trade, or the number of shares if >0
   * @see PositionsManager
   */
  virtual unsigned int onBuyAtStop(size_t bar, unsigned int shares,
                                   double price) const {
    return shares;
  };
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful. It could apply a position sizing algorithm and
   * change the number of shares/contracts, or it could block the trade
   * altogether
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param shares Initial number of shares
   * @param price  The limit price
   *
   * @return 0 to block the trade, or the number of shares if >0
   * @see PositionsManager
   */
  virtual unsigned int onBuyAtLimit(size_t bar, unsigned int shares,
                                    double price) const {
    return shares;
  };
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @return true to enable the order to proceed, false to block the order
   */
  virtual bool onSellAtMarket(size_t bar) const { return true; };
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @return true to enable the order to proceed, false to block the order
   */
  virtual bool onSellAtClose(size_t bar) const { return true; };
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param price  The stop price
   * @return true to enable the order to proceed, false to block the order
   */
  virtual bool onSellAtStop(size_t bar, double price) const { return true; };
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param price  The limit price
   * @return true to enable the order to proceed, false to block the order
   */
  virtual bool onSellAtLimit(size_t bar, double price) const { return true; };
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful. It could apply a position sizing algorithm and
   * change the number of shares/contracts, or it could block the trade
   * altogether
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param shares Initial number of shares
   * @return 0 to block the trade, or the number of shares if >0
   * @see PositionsManager
   */
  virtual unsigned int onShortAtMarket(size_t bar, unsigned int shares) const {
    return shares;
  };
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful. It could apply a position sizing algorithm and
   * change the number of shares/contracts, or it could block the trade
   * altogether
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param shares Initial number of shares
   * @return 0 to block the trade, or the number of shares if >0
   * @see PositionsManager
   */
  virtual unsigned int onShortAtClose(size_t bar, unsigned int shares) const {
    return shares;
  };
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful. It could apply a position sizing algorithm and
   * change the number of shares/contracts, or it could block the trade
   * altogether
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param shares Initial number of shares
   * @param price  The stop price
   * @return 0 to block the trade, or the number of shares if >0
   * @see PositionsManager
   */
  virtual unsigned int onShortAtStop(size_t bar, unsigned int shares,
                                     double price) const {
    return shares;
  };
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. Should be overriden in derived classes to
   * do something meaningful. It could apply a position sizing algorithm and
   * change the number of shares/contracts, or it could block the trade
   * altogether
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param shares Initial number of shares
   * @param price  The limit price
   * @return 0 to block the trade, or the number of shares if >0
   * @see PositionsManager
   */
  virtual unsigned int onShortAtLimit(size_t bar, unsigned int shares,
                                      double price) const {
    return true;
  };
  /**
   * Call-back filter method.
   *
   * Does nothing in the base class. hould be overriden in derived classes to do
   * something meaningful
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @return true to enable the order to proceed, false to block the order
   */
  virtual bool onCoverAtMarket(size_t bar) const { return true; };
  /**
   * Call-back filter method.
   *
   * Should be overriden in derived classes to do something meaningful
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @return true to enable the order to proceed, false to block the order
   */
  virtual bool onCoverAtClose(size_t bar) const { return true; };
  /**
   * Does nothing in the base class. Call-back filter method.
   *
   * Should be overriden in derived classes to do something meaningful
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param price  The stop price
   * @return true to enable the order to proceed, false to block the order
   */
  virtual bool onCoverAtStop(size_t bar, double price) const { return true; };
  /**
   * Does nothing in the base class. Call-back filter method.
   *
   * Should be overriden in derived classes to do something meaningful
   *
   * @param bar    The bar on which the trade that triggered this call is to be
   * executed
   * @param price  The limit price
   * @return true to enable the order to proceed, false to block the order
   */
  virtual bool onCoverAtLimit(size_t bar, double price) const { return true; };

  // for stateful filter classes, resets the state
  virtual void reset() {}
};

/**
 * TradingSignal class.
 *
 * Trading signals are generated within a trading system , when an order would
 * be triggered the bar next to the last one in a collection of bars. They are
 * passed to the user in the signal method of signalHandler
 *
 * Objects of this type are created internally and passed to user code as part
 * of signal handling procedures. The user does not need to create instances of
 * or derive from this class.
 *
 * @see PositionsManager
 * @see signalHandler
 */
class CORE_API Signal {
public:
  /**
   * This enumeration defines the constants passed to the signal method to
   * indicate which type of order triggered the signal
   *
   * @see PositionsManager
   */
  enum SignalType {
    BUY_AT_MARKET = 0,
    BUY_AT_CLOSE,
    BUY_AT_STOP,
    BUY_AT_LIMIT,

    SELL_AT_MARKET,
    SELL_AT_CLOSE,
    SELL_AT_STOP,
    SELL_AT_LIMIT,

    SHORT_AT_MARKET,
    SHORT_AT_CLOSE,
    SHORT_AT_STOP,
    SHORT_AT_LIMIT,

    COVER_AT_MARKET,
    COVER_AT_CLOSE,
    COVER_AT_STOP,
    COVER_AT_LIMIT,
  };

private:
  const SignalType _type;
  const std::string _symbol;
  const DateTime _time;
  const size_t _bar;
  unsigned int _shares;
  const double _price;
  const tradery::Position _pos;
  const std::string _name;
  const std::string _systemName;
  const std::string _systemId;
  const bool _applySignalSizing;

 public:
   // for close limit type order
// in the case, do not apply signal sizing by default (which is done anyway)
   Signal(SignalType type, const std::string& symbol, DateTime time,
     size_t bar, unsigned int shares, double price,
     const tradery::Position pos, const std::string& name,
     const std::string& systemName, const std::string& systemId)
     : _type(type),
     _symbol(symbol),
     _time(time),
     _bar(bar),
     _shares(shares),
     _price(price),
     _pos(pos),
     _name(name),
     _systemName(systemName),
     _applySignalSizing(false),
     _systemId(systemId) {
     assert(!_systemName.empty());
     assert(!_systemId.empty());
   }

   // for open market type order (no price, no position )
   Signal(SignalType type, const std::string& symbol, DateTime time,
     size_t bar, unsigned int shares, const std::string& name,
     const std::string& systemName, bool applySignalSizing,
     const std::string& systemId)
     : _type(type),
     _symbol(symbol),
     _time(time),
     _bar(bar),
     _shares(shares),
     _price(0),
     _name(name),
     _systemName(systemName),
     _applySignalSizing(applySignalSizing),
     _systemId(systemId) {
     assert(!_systemName.empty());
     assert(!_systemId.empty());
   }

   // for close market type order (no price )
   Signal(SignalType type, const std::string& symbol, DateTime time,
     size_t bar, unsigned int shares, const tradery::Position pos,
     const std::string& name, const std::string& systemName,
     const std::string& systemId)
     : _type(type),
     _symbol(symbol),
     _time(time),
     _bar(bar),
     _shares(shares),
     _price(0),
     _pos(pos),
     _name(name),
     _systemName(systemName),
     _applySignalSizing(false),
     _systemId(systemId) {
     assert(!_systemName.empty());
     assert(!_systemId.empty());
   }

   // for open limit type orders ( no position )
   Signal(SignalType type, const std::string& symbol, DateTime time,
     size_t bar, unsigned int shares, double price,
     const std::string& name, const std::string& systemName,
     bool applySignalSizing, const std::string& systemId)
     : _type(type),
     _symbol(symbol),
     _time(time),
     _bar(bar),
     _shares(shares),
     _price(price),
     _name(name),
     _systemName(systemName),
     _applySignalSizing(applySignalSizing),
     _systemId(systemId) {
     assert(!_systemName.empty());
     assert(!_systemId.empty());
   }

   Signal(const Signal& signal)
     : _type(signal.type()),
     _symbol(signal.symbol()),
     _time(signal.time()),
     _bar(signal.bar()),
     _shares(signal.shares()),
     _price(signal.price()),
     _pos(signal.position()),
     _name(signal.name()),
     _systemName(signal.systemName()),
     _applySignalSizing(signal.applySignalSizing()),
     _systemId(signal.systemId()) {
     assert(!_systemName.empty());
     assert(!_systemId.empty());
   }

  /**
   * Static method which returns the string representation of an signal type
   *
   * @param type   The type of the signal
   * @return The string representation of the signal type
   */
  static std::string signalTypeAsString(SignalType type);

 public:
   bool applySignalSizing() const { return _applySignalSizing; }
   /**
   * Creates a duplicate copy of the current signal object
   *
   * @return Pointer to the new signal object
   */
   const std::shared_ptr< Signal > clone() const { return std::make_shared< Signal >(*this); }

  /**
   * Returns the type of the signal, as defined in the enum signalType
   *
   * @return signalType
   */
   SignalType type() const { return _type; }
   /**
   * returns the symbol for which the signal was triggered
   *
   * @return a reference to a string representing the symbol
   */
   const std::string& symbol() const { return _symbol; }
   /**
   * The time signal was triggered
   *
   * @return a XTime object representing the time of the signal
   */
   DateTime time() const { return _time; }
   /**
   * The index of the last bar after which the signal occured
   *
   * @return the bar index
   */
   size_t bar() const { return _bar; }
   /**
   * The number of shares of the signal
   *
   * @return Number of shares
   */
   unsigned int shares() const { return _shares; }
   /**
   * The price of the signal, for orders types the require a price such as limit
   * and stop
   *
   * @return price of the signal
   */
   double price() const { return _price; }
   /**
   * If the signal is to close an existing position (sell, cover), this returns
   * a pointer to that position
   *
   * @return Pointer ot a Position object if the signal is to close the
   * position, 0 otherwise
   */
   const tradery::Position position() const { return _pos; }
   /**
   * The name of the signal, set in the call to the order method that triggered
   * the signal
   *
   * @return a reference to a string representing the name of the signal
   */
   const std::string& name() const { return _name; }
   const std::string& systemName() const { return _systemName; }
   bool isShort() const {
    return _type == SHORT_AT_MARKET || _type == SHORT_AT_CLOSE ||
      _type == SHORT_AT_STOP || _type == SHORT_AT_LIMIT ||
      _type == COVER_AT_MARKET || _type == COVER_AT_CLOSE ||
      _type == COVER_AT_STOP || _type == COVER_AT_LIMIT;
  }

  bool isLong() const { return !isShort(); }
  bool isEntryPosition() const {
    return _type == BUY_AT_MARKET || _type == BUY_AT_CLOSE ||
      _type == BUY_AT_STOP || _type == BUY_AT_LIMIT ||
      _type == SHORT_AT_MARKET || _type == SHORT_AT_CLOSE ||
      _type == SHORT_AT_STOP || _type == SHORT_AT_LIMIT;
  }

  bool isExitPosition() const { return !isEntryPosition(); }
  bool hasPrice() const {
    return _type == BUY_AT_LIMIT || _type == SHORT_AT_LIMIT ||
      _type == BUY_AT_STOP || _type == SHORT_AT_STOP ||
      _type == SELL_AT_LIMIT || _type == COVER_AT_LIMIT ||
      _type == SELL_AT_STOP || _type == COVER_AT_STOP;
  }
  /**
   * The name of the system that triggered the signal
   *
   * @return a reference to a string representing the name of system that
   * triggered the signal
   */
  //  virtual const std::wstring& systemName() const = 0;
  const std::string toCSVString() const;
  static const std::string csvHeaderLine();

  void setShares(unsigned int shares) { _shares = shares; }

  void disable() { _shares = 0; }
  bool isEnabled() const { return _shares > 0; }
  bool isDisabled() const { return !isEnabled(); }
  const std::string& systemId() const { return _systemId; }
};

using SignalPtr = std::shared_ptr<Signal>;
using SignalVector = std::vector<SignalPtr>;

/**
 * \brief Base class for signal handlers
 *
 * A signal handler is a class that can receive trading signal notifications.
 *
 * To use signal handlers in the context of the trading platform, they must be
 * implemented as plug-ins (see the Plugin API), therefore they are derived from
 * Info.
 *
 * In the context of the trading platform, a signal indicates that a trading
 * system has generated an order on the bar next to the last available bar. This
 * includes not only explicit orders methods such as BuyAtMarket etc, but also
 * orders triggered by calls to ApplyAutoStops methods (see PositionsManager)
 *
 * Trading signals can be used to enter trading orders in real-time trading,
 * either manually or automatically, by using an appropriate signal handler
 * implementation (such as one that connects directly to a broker).
 *
 * The class signalHandler is both a default implementation of a signal handler
 * (providing the default behavior which is to ignore the signals) and a base
 * class for all user-defined signal handler proding more useful functionality.
 *
 * The signal method, is called for all signals, regardless of the type of order
 * that triggered it. The type of order is indicated by one of its parameters.
 *
 * <!-- \htmlonly <FONT COLOR="RED">TODO: add more parameters to the signal
 * method, or even break it down into more signals methods by order that
 * triggered them </FONT>\endhtmlonly -->
 *
 * Note: signals are only triggered if at least one signalHandler is registered
 * with a PositionsManager object (using
 * PositionsManager::registersignalHandler). If no signalHandler object is
 * registered, an order past the last bar will throw an exception. Even if a
 * signal handler is registered, exceptions will still be thrown if attempts are
 * made to open/close positions on bars past the (last + 1). For more details,
 * refer to PositionsManager
 *
 * signals can be used in real time trading to send the actual orders into an
 * automatic trading system, or for an End Of Day system, to create a list of
 * orders to be enterd the next day.
 *
 * In order to receive signals, the user has to derive one or more classes from
 * signalHandler and implement the signal methods.
 *
 * @see PositionsManager
 * @see Signal
 * @see Plugin
 */
class SignalHandler : public PluginConfiguration {
 public:
  /**
   * Constructor taking an Info reference as argument
   *
   * @param info
   */
  SignalHandler(const Info& info) : PluginConfiguration(info) {}

  virtual ~SignalHandler() {}

  /**
   * Method when a signal of any type is triggered by a trading system
   * associated with this signal handler
   *
   * The default implementation ignores the signal
   *
   * @param _signal Reference to a Signal object, which contains the information
   * about the triggered signal
   */
  virtual void signal(SignalPtr _signal) = 0;
};

/**
 * Implements a "less" order relationship between Position objects.
 *
 * Used by sort operations on PositionsContainer objects
 *
 * The user needs to derive a concrete class from PositionLessPredicate in order
 * to sort the contents of a PositionContainer object based on a user defined
 * "less" criterion.
 *
 * @see PositionsContainer
 */
class PositionLessPredicate {
 public:
  /**
   * The less method should return true when pos1 is "less" than pos2, where
   * "less" has a user defined semantics.
   *
   * @param pos1   First position to be tested
   * @param pos2   Second position to be tested
   *
   * @return true if pos1 less than pos2, false otherwise
   */
  virtual bool less(const Position pos1, const Position pos2) const = 0;
};

/**
 * Abstract base class (interface) for position handlers.
 *
 * Position handlers are classes used during back-testing to process positions,
 * either open or closed.
 *
 * A position handler class methods are called for each position, by the forEach
 * or forEachConst method of the class PositionsContainer, if a position handler
 * is registered with the PositionsContainer
 *
 * @see PositionsContainer
 * @see PositionsContainer::forEachPosition
 */
class PositionHandler {
 public:
  /**
   * Called by PositionsContainer::forEach, on each position in the container
   *
   * This method receives a non const Position pointer.
   *
   * The user may override this virtual method in a derived class and implement
   * a specific position handler.
   *
   * @param pos    The current position, non const
   *
   * @see PositionsContainer::forEach
   */
   virtual void onPosition( Position pos) = 0;

  /**
   * Called by PositionsContainer::forEachConst, on each position in the
   * container
   *
   * This method receives a const Position pointer.
   *
   *
   * The user may override this virtual method in a derived class and implement
   * a specific position handler.
   *
   * @param pos    The current position, non const
   *
   * @see PositionsContainer::forEachConst
   */
};

/**
 * Abstract base class for open position handlers.
 *
 * A position handler class methods are called on each open position, by the
 * forEachOpenPosition method of the class PositionsContainer or
 * PositionsManager if an open position handler is registered with the
 * PositionsContainer
 *
 * The user should implement specific open position handlers derived from this
 * class and pass instances to the method forEachOpenPosition in the clas
 * PositionsContainer or PositionsManager classes
 *
 * @see PositionsManager
 * @see PositionsContainer
 * @see PositionsContainer::forEachOpenPosition
 * @see PositionsManager::forEachOpenPosition
 */
class OpenPositionHandler {
 public:
  /**
   * Called by PositionsContainer::forEachOpenPosition, on each open position in
   * the container
   *
   * This method receives a Position , as well as other parameters needed to
   * implement the logic for handling open position in a system
   *
   * The user may override this virtual method in a derived class and implement
   * a specific open position handler.
   *
   * @param pos    The position to process
   * @param bars   The collection of bars on which the system is run
   * @param bar    The current bar
   */
  virtual bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) = 0;
};

class OpenPositionHandler1 {
 public:
  /**
   * Called by PositionsContainer::forEachOpenPosition, on each open position in
   * the container
   *
   * This method receives a const Position pointer, as well as other parameters
   * needed to implement the logic for handling open position in a system
   *
   * The user may override this virtual method in a derived class and implement
   * a specific open position handler.
   *
   * @param pos    The position to process
   * @param bars   The collection of bars on which the system is run
   * @param bar    The current bar
   */
  virtual bool onOpenPosition(tradery::Position pos) = 0;
};

/**
 * Abstract base class for predicates used by PositionsContainer::forEach
 * methods
 *
 * A predicate represents a logical condition that is true or false, based on a
 * user defined criteria.
 *
 * For example PositionEqualLongPredicate will yield true for long positions
 * only
 *
 * @see PositionsContainer
 */
class PositionEqualPredicate {
 public:
  virtual ~PositionEqualPredicate() {}

  /**
   * Pure virtual operator== which implements the predicate logical condition.
   *
   * Derived classes will implement this operator to return true if the position
   * passed as parameter meets the required criteria
   *
   * @param position A position to be tested against the logical condition
   *
   * @return true if the position meets the codition, false if not
   */
  virtual bool operator==(const Position position) const = 0;
  /**
   * Operator!= which implements the predicate negated logical condition.
   *
   * Derived classes will implement this operator to return true if the position
   * passed as parameter doesn't meet the required criteria
   *
   * @param position A position to be tested against the logical condition
   *
   * @return true if the position doesn't meet the codition, false if it meets
   * the condition
   */
  bool operator!=(const Position position) const {
    return !operator==(position);
  }
};

class PositionEqualAllPredicate : public PositionEqualPredicate {
  /**
   * Pure virtual operator== which implements the predicate logical condition.
   *
   * Derived classes will implement this operator to return true if the position
   * passed as parameter meets the required criteria
   *
   * @param position A position to be tested against the logical condition
   *
   * @return true if the position meets the codition, false if not
   */
 public:
  virtual bool operator==(const Position position) const { return true; }
};

/**
 * A concrete implementation of a PositionEqualPrediacat which is true for short
 * positions only
 */
class PositionEqualShortPredicate : public PositionEqualPredicate {
 public:
  /**
   * Returns true if the position is short
   *
   * @param position The position to be tested
   *
   * @return true if the position is short, false otherwise
   */
  virtual bool operator==(const Position position) const {
    return position.isShort();
  }
};

/**
 * A concrete implementation of a PositionEqualPrediacat which is true for long
 * positions only
 */
class PositionEqualLongPredicate : public PositionEqualPredicate {
 public:
  /**
   * Returns true if the position is long
   *
   * @param position The position to be tested
   *
   * @return true if the position is long, false otherwise
   */
  virtual bool operator==(const Position position) const {
    return position.isLong();
  }
};

/**
 * A class that is both a PositionEqualPredicate as well as a PositionHandler.
 *
 * It provides a convenient base for classes that need to implement both
 *
 * @see PositionsContainer::forEach
 */
class PositionEqualPredHandler : public PositionEqualPredicate, public PositionHandler {};

/**
 * A container of pointers to positions (open or closed)
 *
 * <!-- \htmlonly <FONT COLOR="RED">TODO: add traversing methods for the ptr
 * list using maybe iterators and function objects. Also for open positions,
 * which will enforce the order of closing of orders</FONT> \endhtmlonly -->
 *
 * PositionsContainer objects are usually associated with a PositionsManager
 * object. The PositionsManager will populate the collection with Position
 * objects.
 *
 * The separation between the PositionsManager and PositionsContainer provides
 * the programmer with a high degree of flexibility. A positions container can
 * be associated with more than one PositionsManager objects simultanously (care
 * must be taken as PositionsContainer is not thread safe, for performance
 * reasons), or at different times.
 *
 * Once all the positions have been created and closed, the PositionsContainer
 * can be handled separately from the PositionsManager. They can be attached to
 * other PositionsContainers, can be sorted using arbitrary criteria etc.
 *
 * All positions in a container can be processed by using a PositionHandler in a
 * call to forEach.
 *
 * A PositionsContainer also manages internally a list of all open positions,
 * optimized for access speed. Open positions can be accessed using the
 * forEachOpenPositions method together with an OpenPositionsHandler.
 *
 * The user can create instances of the PositionsContainer using the static
 * <B>PositionsContainer::create</B> method. The objects thus obtained must be
 * deallocated by the user when not needed any more, by either using the delete
 * operator or by means of "smart" pointers.
 *
 * A user cannot and does not need to directly add positions to objects of this
 * type. They are populated by calling different methods in PositionsManager
 * objects (see PositionsManager constructors and methods) to which the
 * PositionsPtrList objects have been attached (see PositionsManager
 * constructors).
 *
 * A system that manages multiple open positions should traverse them using the
 * methods for open positions only for performance reasons.
 *
 * Note: The positions are owned by the list and deleted by the list destructor
 * - the user does not need to delete them. Thus, the user should not rely on
 * their being around after the container has been destroyed.
 *
 * Note: This class is not thread safe, mainly for performance reasons. Normally
 * one instance of this class should be used by one system instance running in
 * one thread. If multiple instances of the same system are to be run
 * simultanously and all positions are to be generated in one container, then
 * they can be appended after the run ends, with the same result, but with the
 * benefit that speed is not affected by the extra checks required to make a
 * clas thread safe
 *
 * @see PositionsManager
 * @see Position
 */
class PositionsContainer {
 public:
   /**
  * A "smart" pointer to a PositionsContainer object
  *
  * @see PositionsContainer
  */
   using PositionsContainerPtr = std::shared_ptr<PositionsContainer>;

   virtual ~PositionsContainer() {}
  /**
   * \brief Static member called to create instances of PositionsContainer.
   *
   * The user code is the owner of the newly created pointer, so it will need to
   * destroy them too when not needed any more, by using the delete operator or
   * "smart" pointers.
   *
   * @return A pointer to an empty PositionContainer type object
   */
  CORE_API static PositionsContainerPtr create();
  CORE_API static PositionsContainerPtr create(const PositionsContainerPtr pc);

  /**
   * Loops through all positions in the container.
   *
   * For each position in the container it will call the method
   * PositionHandler::onPosition.
   *
   * This method can be used to traverse all the positions in the container and
   * modify them (closing them for example).
   *
   * For a constant version, that doesn't allow modifying positions, use
   * PositionsContainer::forEachConst instead, which will enforce the positions
   * "constness"
   *
   * It is equivalent to a for loop that traverses all the positions in the
   * container, but with the advantage that it makes the code clearer and helps
   * achieve modularity and code reuse.
   *
   * @param onPosition A reference to a PositionHandler object
   *
   * @see PositionsContainer::forEachConst
   * @see PositionsContainer::forEachOpen
   * @see PositionHandler
   */
  virtual void forEach(PositionHandler& onPosition) = 0;
  /**
   * Loops through all positions in the container that meet the criteria defined
   * by the PositionEqualPredicate passed as parameter
   *
   * For each position in the container that meets the criteria it will call the
   * method  PositionHandler::onPosition.
   *
   * This method can be used to traverse all the positions that meet a certain
   * criteria in the container and modify them (closing them for example).
   *
   * It is equivalent to a for loop that traverses all the positions in the
   * container that meet a defined criteria, but with the advantage that it
   * makes the code clearer and helps achieve modularity and code reuse.
   *
   * @param onPosition A reference to a PositionHandler object
   * @param pred       The predicate defining the condtion to be met by each
   * position
   *
   * @see PositionHandler
   * @see PositionEqualPredicate
   */
  virtual void forEach(PositionHandler& onPosition, const PositionEqualPredicate& pred) = 0;
  /**
   * Loops through all positions in the container that don't meet the criteria
   * defined by the PositionEqualPredicate passed as parameter
   *
   * For each position in the container that doesn't meet the criteria it will
   * call the method  PositionHandler::onPosition.
   *
   * This method can be used to traverse all the positions that don't meet a
   * certain criteria in the container and modify them (closing them for
   * example).
   *
   * It is equivalent to a for loop that traverses all the positions in the
   * container that meet a defined criteria, but with the advantage that it
   * makes the code clearer and helps achieve modularity and code reuse.
   *
   * @param onPosition A reference to a PositionHandler object
   * @param pred       The predicate defining the condtion not to be met by each
   * position
   *
   * @see PositionHandler
   * @see PositionEqualPredicate
   */
  virtual void forEachNot(PositionHandler& onPosition, const PositionEqualPredicate& pred) = 0;
  /**
   * Loops through all positions in the container that meet the criteria defined
   * by the PositionEqualPredHandler passed as parameter
   *
   * For each position in the container that meets the criteria it will call the
   * method  PositionEqualPredHandler::onPosition.
   *
   * This method can be used to traverse all the positions that meet a certain
   * criteria in the container and modify them (closing them for example).
   *
   * It is equivalent to a for loop that traverses all the positions in the
   * container that meet a defined criteria, but with the advantage that it
   * makes the code clearer and helps achieve modularity and code reuse.
   *
   * This method is similar to the forEach method that takes a PositionHandler
   * and a PositionEqualPredicate, with the difference that the parameter
   * implements both a handler and predicate
   *
   * @param predHandler A reference to a PositionEqualPredHandler object
   *
   * @see PositionHandler
   * @see PositionEqualPredicate
   * @see PositionEqualPredHandler
   */
  virtual void forEach(PositionEqualPredHandler& predHandler) = 0;
  /**
   * Loops through all positions in the container that don't meet the criteria
   * defined by the PositionEqualPredHandler passed as parameter
   *
   * For each position in the container that doens't meet the criteria it will
   * call the method  PositionEqualPredHandler::onPosition.
   *
   * This method can be used to traverse all the positions that don't meet a
   * certain criteria in the container and modify them (closing them for
   * example).
   *
   * It is equivalent to a for loop that traverses all the positions in the
   * container that don't meet a defined criteria, but with the advantage that
   * it makes the code clearer and helps achieve modularity and code reuse.
   *
   * This method is similar to the forEachNot method that takes a
   * PositionHandler and a PositionEqualPredicate, with the difference that the
   * parameter implements both a handler and predicate
   *
   * @param predHandler A reference to a PositionEqualPredHandler object
   *
   * @see PositionHandler
   * @see PositionEqualPredicate
   * @see PositionEqualPredHandler
   */
  virtual void forEachNot(PositionEqualPredHandler& predHandler) = 0;
  /**
   * Loops through all positions in the container that meet at least one of a
   * criteria defined by the vector of PositionEqualPredicate passed as
   * parameter
   *
   * For each position in the container that meets at least one criteria, it
   * will call the method  PositionHandler::onPosition.
   *
   * This method can be used to traverse all the positions that meet a criteria
   * out of a set of criteria in the container and modify them (closing them for
   * example).
   *
   * It is equivalent to a for loop that traverses all the positions in the
   * container that meet at least a one criteria out of a set of criteria, but
   * with the advantage that it makes the code clearer and helps achieve
   * modularity and code reuse.
   *
   * @param positionHandler
   *                   The handler called for positions that meet at least on
   * condition
   * @param predicates A vector of pointer to predicates
   *
   * @see PositionHandler
   * @see PositionEqualPredicate
   */
  virtual void forEachOr(PositionHandler& positionHandler, std::vector<PositionEqualPredicate*> predicates) = 0;
  /**
   * Loops through all positions in the container that meet all the criteria
   * criteria defined by the vector of PositionEqualPredicate passed as
   * parameter
   *
   * For each position in the container that meets all the criteria, it will
   * call the method  PositionHandler::onPosition.
   *
   * This method can be used to traverse all the positions that meet all the
   * criteria in a set of criteria in the container and modify them (closing
   * them for example).
   *
   * It is equivalent to a for loop that traverses all the positions in the
   * container that meet all the criteria in a set of criteria, but with the
   * advantage that it makes the code clearer and helps achieve modularity and
   * code reuse.
   *
   * @param positionHandler
   *                   The handler called for positions that meet at least on
   * condition
   * @param predicates A vector of pointer to predicates
   *
   * @see PositionHandler
   * @see PositionEqualPredicate
   */
  virtual void forEachAnd(PositionHandler& positionHandler, std::vector<PositionEqualPredicate*> predicates) = 0;
  /**
   * Similar to forEach, it runs a loop on all positions in the container, but
   * with each position constant
   *
   * For each position in the container, it will call the method
   * PositionHandler::onPositionConst
   *
   * This method is const so it will not allow changing the position (closing it
   * for example).
   *
   * This can be used to traverse all positions.
   *
   * If it is necessary to change the positions, use PositionsContainer::forEach
   * instead.
   *
   * It is equivalent to a for loop that traverses all the positions in the
   * container, but with the advantage that it makes the code clearer and helps
   * achieve modularity and code reuse.
   *
   * @param onPosition A reference to an PositionHandler object
   *
   * @see PositionsContainer::forEach
   * @see PositionsContainer::forEachOpen
   * @see PositionHandler
   */
  virtual void forEachConst(PositionHandler& onPosition) const = 0;
  virtual void forEachConst(PositionHandler& onPosition, PositionEqualPredicate& pred) const = 0;
  /**
   * Runs a loop on all closed positions in the container
   *
   * <!-- TODO: this is probably not necessary - closed positions cannot be
   * modified --> For each closed position in the container, it will call the
   * method  PositionHandler::onPosition
   *
   * This can be used to traverse all closed positions.
   *
   * It is equivalent to a for loop that traverses all the closed positions in
   * the container, but with the advantage that it makes the code clearer and
   * helps achieve modularity and code reuse.
   *
   * @param opPosition
   *
   * @see PositionsContainer::forEach
   * @see PositionsContainer::forEachOpen
   * @see PositionHandler
   */
  virtual void forEachClosed(PositionHandler& opPosition) = 0;
  /**
   * \brief Similar to forEachClosedPositions, with the difference that each
   * position is constant
   *
   * For each closed position in the container, it will call the method
   * PositionHandler::onPositionConst
   *
   * This can be used to traverse all closed positions.
   *
   * It is equivalent to a for loop that traverses all the closed positions in
   * the container, but with the advantage that it makes the code clearer and
   * helps achieve modularity and code reuse.
   *
   * @param opPosition
   *
   * @see PositionsContainer::forEach
   * @see PositionsContainer::forEachOpen
   * @see PositionHandler
   */
  virtual void forEachClosedConst(PositionHandler& opPosition) const = 0;
  /**
   * Runs a loop on all open positions in the container
   *
   * For each closed position in the container, it will call the method
   * OpenPositionHandler::onOpenPosition
   *
   * This can be used to traverse all open positions and/or close some or all of
   * them.
   *
   * It is equivalent to a for loop that traverses all the open positions in the
   * container, but with the advantage that it makes the code clearer and helps
   * achieve modularity and code reuse.
   *
   * @param openPositionHandler
   * @param bars
   * @param bar
   *
   * @see PositionsContainer::forEach
   * @see PositionsContainer::forEachClosed
   * @see PositionHandler
   */
  virtual void forEachOpenPosition(OpenPositionHandler& openPositionHandler, Bars bars, size_t bar) = 0;
  virtual void forEachOpenPosition(OpenPositionHandler1& openPositionHandler) = 0;
  virtual void forEachOpenPosition(OpenPositionHandler1& openPositionHandler, const PositionEqualPredicate& pred) = 0;
  //  virtual void forEachOpenCloseIf( ClosePredicate& pr, size_t bar ) = 0;

  /**
   * Adds the contents of another PositionsContainer to the end of the current
   * container.
   *
   * The source PositionsContainer will be emptied as a result of this call.
   *
   * @param posList The PositionsContainer object whose contents is to be
   * appended to the end of the current one. Cannot be 0.
   */
  virtual void append(PositionsContainer* posList) = 0;
  virtual void nonDestructiveAppend(PositionsContainer* pc) = 0;
  /**
   * Returns the total number of positions in the container (open or closed)
   *
   * @return Number of positions in the list
   */
  virtual size_t count() const = 0;
  /**
   * Returns the total number of enabled positions in the container
   * This is used in case the positions have been processed by the position
   * sizing class which could disable a number of the positions in the container
   */
  virtual size_t enabledCount() const = 0;

  virtual tradery::Position getPosition(PositionId id) = 0;

  /**
   * Returns a pointer to the last Position pointer in the container (open or
   * closed)
   *
   * Note that if the list has been sorted, the last position in the list may
   * not be the last position chronologically that has been  opened
   *
   * @return pointer to the last Position in the list. Returns 0 if the list is
   * empty
   * @see Position
   */
  virtual tradery::Position getLastPosition() = 0;
  /**
   * Returns the number of open positions
   *
   * @return The number of open positions
   * @see Position
   */
  virtual size_t openPositionsCount() const = 0;
  /**
   * Returns the last open position as a pointer to a non-const Position type
   * object
   *
   * @return non-const pointer to the last open position. 0 if no open positions
   * @see Position
   */
  virtual tradery::Position getLastOpenPosition() = 0;
  /**
   * Returns the last open position as a pointer to a const Position type object
   *
   * @return const pointer to the last open position. 0 if no open positions
   * @see Position
   */
  virtual const tradery::Position getLastOpenPosition() const = 0;
  /**
   * Empties the container by removing all the positions, open or closed
   */
  virtual void clear() = 0;
  /**
   * General sort method, that takes a user defined comparison predicate
   * PositionLessPredicate as parameter
   *
   * Sorting can be done in any possible order, given the right predicate,
   * including sorting by multiple fields, or even randomization
   *
   * @param predicate Comparison predicate
   * @param ascending true for ascending sorting, false for descending sorting,
   * default ascending
   * @see PositionGreaterPredicate
   */
  virtual void sort(PositionLessPredicate& predicate, bool ascending = true) = 0;
  /**
   * Sorts the container contents by entry time
   *
   * @param ascending true for ascending, false for descending, default
   * ascending
   * @see sort
   */
  virtual void sortByEntryTime(bool ascending = true) = 0;
  /**
   * Sorts container contents by exit time.
   *
   * An open position is considered less than a closed position
   *
   * @param ascending true for ascending, false for descending, default
   * ascending
   * @see sort
   */
  virtual void sortByExitTime(bool ascending = true) = 0;
  /**
   * Sorts container contents by position gain (exit price - entry price)
   *
   * Open position is less than a closed position
   *
   * @param ascending true for ascending, false for descending, default
   * ascending
   *
   * @see sort
   */
  virtual void sortByGain(bool ascending = true) = 0;
  /**
   * Reverses the order of all positions in the container.
   */
  virtual void reverse() = 0;
};

class PositionsPtrVector
    : public std::vector<PositionsContainer::PositionsContainerPtr> {
 private:
  std::mutex _m;

 public:
  void add(const PositionsContainer::PositionsContainerPtr pc) {
    std::scoped_lock lock(_m);
    __super::push_back(pc);
  }

  size_t enabledCount() const {
    size_t enabledCount = 0;

    for (auto v : *this) {
      enabledCount += v->enabledCount();
    }

    return enabledCount;
  }

  bool hasEnablePositions() const { return enabledCount() > 0; }
};

class PositionsIteratorAbstr {
 public:
   using PositionsIteratorAbstrPtr = std::shared_ptr< PositionsIteratorAbstr >;
  static PositionsIteratorAbstrPtr create(const PositionsContainer::PositionsContainerPtr pc);
  virtual Position first() = 0;
  virtual Position next() = 0;
  /*
  virtual Position operator++() = 0;
  virtual Position operator++( int ) = 0;
  */
  virtual void reset() = 0;
};

class PositionsIteratorConst {
 private:
  std::shared_ptr<PositionsIteratorAbstr> _pi;

 public:
  PositionsIteratorConst(PositionsContainer::PositionsContainerPtr pc)
      : _pi(PositionsIteratorAbstr::create(pc)) {}

  const Position first() { return _pi->first(); }
  const Position next() { return _pi->next(); }
  /*    const Position operator++() { return ++(*_pi); }
      const Position operator++( int ) { return (*pi)++; }
      */
  void reset() { _pi->reset(); }
};

class PositionsIterator {
 private:
  std::shared_ptr<PositionsIteratorAbstr> _pi;

 public:
  PositionsIterator(const PositionsContainer::PositionsContainerPtr pc)
      : _pi(PositionsIteratorAbstr::create(pc)) {}

  Position first() { return _pi->first(); }
  Position next() { return _pi->next(); }
  /*    Position operator++() { return ++(*_pi); }
      Position operator++( int ) { return (*pi)++; }
      */
  void reset() { _pi->reset(); }
};

using PositionsContainerVector = std::vector<PositionsContainer::PositionsContainerPtr >;

class PositionsVector : public PositionsContainerVector {
 private:
  PositionsContainer::PositionsContainerPtr _all;
  mutable std::mutex _mx;

 public:
  PositionsVector() : _all(PositionsContainer::create()) {}

  PositionsContainer::PositionsContainerPtr getNewPositionsContainer() {
    std::scoped_lock lock(_mx);
    PositionsContainer::PositionsContainerPtr p = PositionsContainer::create();
    push_back(p);
    return p;
  }

  PositionsContainer* getAllPositions() {
    std::scoped_lock lock(_mx);
    return _all.get();
  }

  // this moves all positions to _all (
  PositionsContainer* populateAllPositions() {
    std::scoped_lock lock(_mx);
    for (auto v : *this) {
      _all->nonDestructiveAppend(v.get());
    }

    return _all.get();
  }

  size_t count() const {
    std::scoped_lock lock(_mx);
    size_t size = 0;

    for (auto v : *this) {
      size += v->count();
    }
    return size;
  }

  size_t enabledCount() const {
    std::scoped_lock lock(_mx);
    size_t size = 0;

    for (auto v : *this) {
      size += v->enabledCount();
    }
    return size;
  }
};

class PositionFormatBase : public PositionHandler {
 protected:
  std::ostream& _os;
  bool _empty;
  unsigned __int64 _maxLines;
  unsigned __int64 _count;

 protected:
  unsigned __int64 count() const { return _count; }

 public:
  /**
   *
   * @param os
   * @param maxLines The maximum number of lines to be generated. All lines are
   * generated if 0
   */
  PositionFormatBase(std::ostream& os) : _os(os), _empty(true), _count(0) {}

  virtual ~PositionFormatBase() {}

  void onPosition(Position pos) override {
    if (_empty) {
      header();
      _empty = false;
    }
    assert(pos);
    line(pos);
    _count++;
  }

  virtual void header() const = 0;
  virtual void line(const Position pos) const = 0;
  virtual void footer() const = 0;

  bool empty() const { return _empty; }
};

constexpr auto TAB = "\t";
constexpr auto TD_OPEN = "<td class=\"c\">";
constexpr auto TD_OPEN_NOWRAP = "<td nowrap class=\"c\">";
constexpr auto TD_CLOSE = "</td>";

// classes:
// tr - d0/d1 : even/odd rows
// td - l/w : loser/winner
// td - c : cell
// td - n/p : negative number/positive number
// td - sh/lg : short/long
//

class PositionToHTMLFormat : public PositionFormatBase {
 private:
  const bool _dateOnly;
  std::ostream& _desc;
  size_t _linesPerPage;

 public:
  // dateOnly indicates if we'are showing the date and time, or date only in
  // start/end time columns
  PositionToHTMLFormat(std::ostream& os, std::ostream& desc, size_t linesPerPage = 25, bool dateOnly = true)
      : PositionFormatBase(os),
        _dateOnly(dateOnly),
        _desc(desc),
        _linesPerPage(linesPerPage) {}

  virtual void header() const {
    std::string dtTitle = _dateOnly ? "date" : "date/time";

    _desc << "header=" << "<tr class=\"h\"> <td class=\"h\">Long/ Short</td> <td class=\"h\">Symbol</td> <td class=\"h\">Shares</td> <td class=\"h\">Entry "
          << dtTitle
          << "</td> <td class=\"h\">Entry price</td> <td class=\"h\">Entry name</td> <td class=\"h\">Exit "
          << dtTitle
          << "</td> <td class=\"h\">Exit price</td> <td class=\"h\">Exit name</td> <td class=\"h\">Gain</td> <td class=\"h\">Gain %</td> <td class=\"h\">System</td></tr>"
          << std::endl;
  }
  virtual void line(const Position pos) const {
    DateTime st = pos.getEntryTime();

    std::string entryDateTime = _dateOnly ? st.date().to_simple_string() : st.to_simple_string();
    std::string closeDateTime;
    if (!pos.isOpen()) {
      DateTime et = pos.getCloseTime();
      closeDateTime = _dateOnly ? et.date().to_simple_string() : et.to_simple_string();
    }
    else {
      closeDateTime = "---";
    }

    std::string gainclass;
    if (!pos.isOpen()) {
      gainclass = pos.getGain() < 0 ? " l" : " w";
    }
    else {
      gainclass = " o";
    }

    std::string title = pos.isLong() ? "alt='Long' title='Long'" : "alt='Short' title='Short'";

    if ((_count % _linesPerPage) == 0) {
      _desc << "line=" << _count << "," << _os.tellp() << std::endl;
    }

    std::string rowClass = __super::count() % _linesPerPage % 2 ? "d0" : "d1";

    _os << "<tr class=\"" << rowClass << gainclass << "\">";
    _os << "<td class=\"c" << (pos.isLong() ? " lg" : " sh") << "\">" << TD_CLOSE;
    // wrapping the symbol between * * se we can replace it with a link to the
    // chart
    _os << TD_OPEN << "*" << pos.getSymbol() << "*" << TD_CLOSE;
    _os << TD_OPEN << pos.getShares() << TD_CLOSE;
    _os << TD_OPEN_NOWRAP << entryDateTime << TD_CLOSE;
    _os << TD_OPEN << std::fixed << std::setprecision(2) << pos.getEntryPrice() << TD_CLOSE;
    _os << TD_OPEN << pos.getEntryName() << TD_CLOSE;
    _os << TD_OPEN_NOWRAP << (pos.isOpen() ? "---" : closeDateTime) << TD_CLOSE;

    _os << TD_OPEN;
    if (pos.isOpen()) {
      _os << "---";
    }
    else {
      _os << std::fixed << std::setprecision(2) << pos.getClosePrice();
    }
    _os << TD_CLOSE;

    if (pos.isOpen()) {
      _os << TD_OPEN << "---</td>" << TD_OPEN << "---</td>" << TD_OPEN << "---</td>";
    }
    else {
      std::string c = pos.getGain() > 0 ? "c p" : "c n";

      _os << "<td class=\"c\">" << pos.getCloseName() << TD_CLOSE;
      _os << "<td nowrap class=\"" << c << "\">" << std::fixed << std::setprecision(2) << pos.getGain() << TD_CLOSE;
      _os << "<td nowrap class=\"" << c << "\">" << std::fixed << std::setprecision(2) << pos.getPctGain() << "%" << TD_CLOSE;
    }

    assert(!pos.getUserString().empty());
    _os << TAB << TD_OPEN << pos.getUserString() << TD_CLOSE;

    _os << "</tr>" << std::endl;
  }

  virtual void footer() const {
    _desc << "end=" << _count << "," << _os.tellp() << std::endl;
  }
};

class PositionToCSVFormat : public PositionFormatBase {
 public:
  PositionToCSVFormat(std::ostream& os) : PositionFormatBase(os) {
    LOG(log_info, "[PositionToCSVFormat] constructor");
  }
  virtual void header() const {
    _os << "Symbol,Shares,Entry time,Entry bar,Entry price,Entry "
           "slippage,Entry commission,Entry name,Exit time,Exit bar,Exit "
           "price,Exit slippage,Exit commission,Exit name, Gain, System name"
        << std::endl;
  }
  virtual void line(const Position pos) const {
    char sep = ',';

    _os << pos.getSymbol() << sep << pos.getShares() << sep
        << pos.getEntryTime().to_simple_string() << sep << pos.getEntryBar()
        << sep << std::fixed << std::setprecision(2) << pos.getEntryPrice()
        << sep << pos.getEntrySlippage() << sep << pos.getEntryCommission()
        << sep << pos.getEntryName();
    if (pos.isClosed()) {
      _os << sep << pos.getCloseTime().to_simple_string() << sep
          << pos.getCloseBar() << sep << std::fixed << std::setprecision(2)
          << pos.getClosePrice() << sep << pos.getCloseSlippage() << sep
          << pos.getCloseCommission() << sep << pos.getCloseName() << sep
          << pos.getGain();
    }
    else {
      _os << sep << sep << sep << sep << sep << sep << sep;
    }

    _os << sep << pos.getUserString();

    _os << std::endl;
  }

  virtual void footer() const {}
};

class PositionsContainerToFormat {
 public:
  PositionsContainerToFormat(const PositionsContainer& pc, PositionFormatBase&& pf, unsigned __int64 maxLines = 0) {
    pc.forEachConst(pf);
    if (!pf.empty()) {
      pf.footer();
    }
  }

  PositionsContainerToFormat(const PositionsVector& pcs, PositionFormatBase&& pf, unsigned __int64 maxLines = 0) {
    for (auto v : pcs) {
      v->forEachConst(pf);
    }
    if (!pf.empty()) {
      pf.footer();
    }
  }
};

class PositionsContainerToHTML : public PositionsContainerToFormat {
 public:
  // lines to generate
  // all lines if 0
  PositionsContainerToHTML(const PositionsContainer& pc, std::ostream& os, std::ostream& desc, size_t linesPerPage)
      : PositionsContainerToFormat(pc, PositionToHTMLFormat(os, desc, linesPerPage)) {}

  // lines to generate
  // all lines if 0
  PositionsContainerToHTML(const PositionsVector& pcs, std::ostream& os, std::ostream& desc, size_t linesPerPage)
      : PositionsContainerToFormat(pcs, PositionToHTMLFormat(os, desc, linesPerPage)) {}
};

class PositionsContainerToCSV : public PositionsContainerToFormat {
 public:
  PositionsContainerToCSV(const PositionsContainer& pc, std::ostream& os)
      : PositionsContainerToFormat(pc, PositionToCSVFormat(os)) {}

  PositionsContainerToCSV(const PositionsVector& pcs, std::ostream& os)
      : PositionsContainerToFormat(pcs, PositionToCSVFormat(os)) {}
};

class PositionIdNotFoundException : public SystemException {
 private:
  PositionId _id;

 public:
  PositionIdNotFoundException(PositionId id)
      : _id(id), SystemException(POSITION_ID_NOT_FOUND_ERROR, message()) {}

  const std::string message() const {
    return tradery::format("Could not find position with id ", _id);
  }
};

class PositionsI {
 public:
  virtual ~PositionsI() {}
  virtual Position getPosition(PositionId id) = 0;
  virtual void forEachOpenPosition(OpenPositionHandler& openPositionHandler, Bars bars, size_t bar) = 0;
  virtual size_t positionCount() const = 0;
  virtual size_t openPositionsCount() const = 0;
  virtual void installTimeBasedExit(Index bars) = 0;
  virtual void installTimeBasedExitAtMarket(Index bars) = 0;
  virtual void installTimeBasedExitAtClose(Index bars) = 0;
  virtual void installBreakEvenStop(double level) = 0;
  virtual void installBreakEvenStopLong(double level) = 0;
  virtual void installBreakEvenStopShort(double level) = 0;
  virtual void installReverseBreakEvenStop(double level) = 0;
  virtual void installReverseBreakEvenStopLong(double level) = 0;
  virtual void installReverseBreakEvenStopShort(double level) = 0;
  virtual void installStopLoss(double level) = 0;
  virtual void installProfitTarget(double level) = 0;
  virtual void installProfitTargetLong(double level) = 0;
  virtual void installProfitTargetShort(double level) = 0;
  virtual void installTrailingStop(double trigger, double lossLevel) = 0;
  virtual void applyTimeBased(Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyTimeBasedAtMarket( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyTimeBasedAtClose( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyStopLoss(Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyTrailing(Bars, size_t barIndex, Position pos) = 0;
  virtual void applyBreakEvenStop( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyReverseBreakEvenStop( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyProfitTarget(Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyProfitTargetLong( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyProfitTargetShort( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyAutoStops(Bars bars, size_t barIndex) = 0;
  virtual PositionId buyAtMarket(Bars bars, size_t barIndex, size_t shares, const std::string& name) = 0;
  virtual PositionId buyAtClose( Bars bars, size_t barIndex, size_t shares, const std::string& name) = 0;
  virtual PositionId buyAtStop( Bars bars, size_t barIndex, double stopPrice, size_t shares, const std::string& name) = 0;
  virtual PositionId buyAtLimit( Bars bars, size_t barIndex, double limitPrice, size_t shares, const std::string& name) = 0;
  virtual bool sellAtMarket( Bars bars, size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool sellAtMarket( Bars bars, size_t barIndex, PositionId pos, const std::string& name) = 0;
  virtual bool sellAtClose(Bars bars, size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool sellAtClose(Bars bars, size_t barIndex, PositionId pos, const std::string& name) = 0;
  virtual bool sellAtStop( Bars bars, size_t barIndex, Position pos, double stopPrice, const std::string& name) = 0;
  virtual bool sellAtStop( Bars bars, size_t barIndex, PositionId pos, double stopPrice, const std::string& name) = 0;
  virtual bool sellAtLimit( Bars bars, size_t barIndex, Position pos, double limitPrice, const std::string& name) = 0;
  virtual bool sellAtLimit( Bars bars, size_t barIndex, PositionId pos, double limitPrice, const std::string& name) = 0;
  virtual PositionId shortAtMarket( Bars bars, size_t barIndex, size_t shares, const std::string& name) = 0;
  virtual PositionId shortAtClose( Bars bars, size_t barIndex, size_t shares, const std::string& name) = 0;
  virtual PositionId shortAtStop( Bars bars, size_t barIndex, double stopPrice, size_t shares, const std::string& name) = 0;
  virtual PositionId shortAtLimit( Bars bars, size_t barIndex, double limitPrice, size_t shares, const std::string& name) = 0;
  virtual bool coverAtMarket( Bars bars, size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool coverAtMarket( Bars bars, size_t barIndex, PositionId pos, const std::string& name) = 0;
  virtual bool coverAtClose( Bars bars, size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool coverAtClose( Bars bars, size_t barIndex, PositionId pos, const std::string& name) = 0;
  virtual bool coverAtStop( Bars bars, size_t barIndex, Position pos, double stopPrice, const std::string& name) = 0;
  virtual bool coverAtStop( Bars bars, size_t barIndex, PositionId pos, double stopPrice, const std::string& name) = 0;
  virtual bool coverAtLimit( Bars bars, size_t barIndex, Position pos, double limitPrice, const std::string& name) = 0;
  virtual bool coverAtLimit( Bars bars, size_t barIndex, PositionId pos, double limitPrice, const std::string& name) = 0;
  virtual void closeAllAtMarket( Bars bars, size_t barIndex, const std::string& name) = 0;
  virtual void closeAllShortAtMarket( Bars bars, size_t barIndex,const std::string& name) = 0;
  virtual void closeAllLongAtMarket( Bars bars, size_t barIndex, const std::string& name) = 0;
  virtual void closeAllAtClose( Bars bars, size_t barIndex, const std::string& name) = 0;
  virtual void closeAllShortAtClose( Bars bars, size_t barIndex, const std::string& name) = 0;
  virtual void closeAllLongAtClose( Bars bars, size_t barIndex, const std::string& name) = 0;
  virtual void closeAllShortAtLimit( Bars bars, size_t barIndex, double price, const std::string& name) = 0;
  virtual void closeAllLongAtLimit( Bars bars, size_t barIndex, double price, const std::string& name) = 0;
  virtual void closeAllShortAtStop( Bars bars, size_t barIndex, double price, const std::string& name) = 0;
  virtual void closeAllLongAtStop( Bars bars, size_t barIndex, double price, const std::string& name) = 0;
  virtual void applyTimeBased(size_t barIndex, Position pos) = 0;
  virtual void applyStopLoss(size_t barIndex, Position pos) = 0;
  virtual void applyTrailing(size_t barIndex, Position pos) = 0;
  virtual void applyBreakEvenStop(size_t barIndex, Position pos) = 0;
  virtual void applyBreakEvenStopLong(size_t barIndex, Position pos) = 0;
  virtual void applyBreakEvenStopShort(size_t barIndex, Position pos) = 0;
  virtual void applyReverseBreakEvenStop(size_t barIndex, Position pos) = 0;
  virtual void applyReverseBreakEvenStopLong( size_t barIndex, Position pos) = 0;
  virtual void applyReverseBreakEvenStopShort( size_t barIndex, Position pos) = 0;
  virtual void applyProfitTarget(size_t barIndex, Position pos) = 0;
  virtual void applyProfitTargetLong(size_t barIndex, Position pos) = 0;
  virtual void applyProfitTargetShort(size_t barIndex, Position pos) = 0;
  virtual void applyAutoStops(size_t barIndex) = 0;
  virtual PositionId buyAtMarket( size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  virtual PositionId buyAtClose( size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  virtual PositionId buyAtStop( size_t barIndex, double stopPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  virtual PositionId buyAtLimit( size_t barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  virtual bool sellAtMarket( size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool sellAtMarket( size_t barIndex, PositionId pos, const std::string& name) = 0;
  virtual bool sellAtClose( size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool sellAtClose( size_t barIndex, PositionId pos, const std::string& name) = 0;
  virtual bool sellAtStop( size_t barIndex, Position pos, double stopPrice, const std::string& name) = 0;
  virtual bool sellAtStop( size_t barIndex, PositionId pos, double stopPrice, const std::string& name) = 0;
  virtual bool sellAtLimit( size_t barIndex, Position pos, double limitPrice, const std::string& name) = 0;
  virtual bool sellAtLimit( size_t barIndex, PositionId pos, double limitPrice, const std::string& name) = 0;
  virtual PositionId shortAtMarket( size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  virtual PositionId shortAtClose( size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  virtual PositionId shortAtStop( size_t barIndex, double stopPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  virtual PositionId shortAtLimit( size_t barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  virtual bool coverAtMarket( size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool coverAtMarket( size_t barIndex, PositionId pos, const std::string& name) = 0;
  virtual bool coverAtClose( size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool coverAtClose( size_t barIndex, PositionId pos, const std::string& name) = 0;
  virtual bool coverAtStop( size_t barIndex, Position pos, double stopPrice, const std::string& name) = 0;
  virtual bool coverAtStop( size_t barIndex, PositionId pos, double stopPrice, const std::string& name) = 0;
  virtual bool coverAtLimit( size_t barIndex, Position pos, double limitPrice, const std::string& name) = 0;
  virtual bool coverAtLimit( size_t barIndex, PositionId pos, double limitPrice, const std::string& name) = 0;
  virtual void closeAllAtMarket(size_t barIndex, const std::string& name) = 0;
  virtual void closeAllShortAtMarket( size_t barIndex, const std::string& name) = 0;
  virtual void closeAllLongAtMarket( size_t barIndex, const std::string& name) = 0;
  virtual void closeAllAtClose(size_t barIndex, const std::string& name) = 0;
  virtual void closeAllShortAtClose( size_t barIndex, const std::string& name) = 0;
  virtual void closeAllLongAtClose( size_t barIndex, const std::string& name) = 0;
  virtual void closeAllShortAtLimit( size_t barIndex, double price, const std::string& name) = 0;
  virtual void closeAllLongAtLimit( size_t barIndex, double price, const std::string& name) = 0;
  virtual void closeAllShortAtStop( size_t barIndex, double price, const std::string& name) = 0;
  virtual void closeAllLongAtStop( size_t barIndex, double price, const std::string& name) = 0;
  virtual tradery::Position getLastPosition() = 0;
};

class OpenPositionsIteratorAbstr {
 public:
  virtual ~OpenPositionsIteratorAbstr() {}

  virtual Position getFirst() = 0;
  virtual Position getNext() = 0;
};

class OpenPositionsIterator : public OpenPositionsIteratorAbstr {
 private:
  std::shared_ptr<OpenPositionsIteratorAbstr> _it;

 public:
  OpenPositionsIterator() {}
  OpenPositionsIterator(std::shared_ptr<OpenPositionsIteratorAbstr> it) : _it(it) {}

  operator bool() const { return (bool)_it; }

  virtual Position getFirst() { return _it->getFirst(); }
  virtual Position getNext() { return _it->getNext(); }
};


/**
 * \brief Manages positions in a PositionsContainer object
 *
 * This class manages all positions entry/exit methods for bar based systems.
 *
 * A PositionsManager must be associated with a PositionsContainer object on
 * which it will apply its positions management methods.
 *
 * The user does not need to derive from this class. Instead, the different
 * <B>create</b> methods should be used to create objects of type
 * PositionsManager
 *
 *
 * The standard implementation of the PositionsManager class contains pointers
 * or references to
 *  - a PositionsContainer type object which is the container for all the
 * created and closed positions - by default 0, needs to be passed in at
 * creation time
 *  - a pointer to a OrderFilter type object - by default it points to a
 * OrderFilter object that allows all orders
 *  - a list of pointers to signalHandler type objects - default an empty list -
 * they need to be registered using registersignalHandler
 *  - a pointer to a Slippage type object - default 0 (no slippage)
 *  - a pointer to a Commission type object - default 0 (no commission)
 *
 * The current PositionsManager implementation does not own any of these
 * pointers, thus they are not deleted by the destructor of the class, so it's
 * the user's responsibility to control their life cycle by either creating them
 * on the stack, or using smart pointers or calling delete when they are no
 * longer necessary
 *
 * Please see PositionsContainer for more details on the separation between the
 * PositionsManager and PositionsContainer
 *
 * @see OrderFilter
 * @see signalHandler
 * @see PositionsContainer
 * @see Commission
 * @see Slippage
 */
class Positions;
class PositionsManagerAbstr {
  friend Positions;

 private:
  virtual PositionId buyAtPrice( Bars bars, size_t barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing, PositionId id) = 0;
  virtual PositionId shortAtPrice(Bars bars, size_t barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing, PositionId id) = 0;

 public:
  using PositionsManagerAbstrPtr = std::shared_ptr<PositionsManagerAbstr>;
  virtual ~PositionsManagerAbstr() {}

  // this is a string that will be associated with all postions created by this
  // manager usually will be used by multisystems to indentify which positions
  // were created by which system
  virtual void setSystemName(const std::string& str) = 0;
  virtual const std::string& systemName() const = 0;

  virtual void setSystemId(const std::string& str) = 0;
  virtual const std::string& systemId() const = 0;

  virtual Position getPosition(PositionId id) = 0;
  /**
   * Traverses the list of all open positions in the associated
   * PositionsContainer.
   *
   * Internally, it calls the forEachOpenPositions on the associated
   * PositionsContainer object
   *
   * @param openPositionHandler
   *               the openPositionHandler to handle each open position
   * @param bars   Can be any bar container, but in general it will be the
   * default bar container for the current system
   * @param bar
   */
  virtual void forEachOpenPosition(OpenPositionHandler& openPositionHandler, Bars bars, size_t bar) = 0;
  /**
   * static member - creates instances of PositionsManager, initialized with a
   * pointer to a PositionsContainer object
   *
   * The PositionsManager type object created with this method contains default
   * values for
   * - OrderFilter (no filtering)
   * - signalHandler (no signal handler)
   * - Slippage (no slippage)
   * - Commission (no commission)
   *
   * The PositionsContainer object must have been created by the user code. The
   * PositionsManager does not own this pointer so this instance will have to be
   * deleted by the user
   *
   * A complex system may need to handle multiple PositionsManagers, each with
   * their own PositionsContainer. Such systems include adaptive systems, which
   * may need temporary PositionsManagers and containers to calculate the
   * optimization parameters.
   *
   * @param posList A pointer to a PositionsContainer object. Cannot be null.
   *
   * @return A pointer to a Position type object
   * @see PositionsContainer
   */
  //  CORE_API static PositionsManagerAbstr* create( PositionsContainer*
  //  posList );
  /**
   * static member - creates instances of PositionsManager, initialized with a
   * pointer to PositionsContainer, Slippage and Commission objects
   *
   * The PositionsManager type object created with this method contains default
   * values for
   * - OrderFilter (no filtering)
   * - signalHandler (no signal handler)
   *
   * The PositionsContainer, Slippage and Commission objects must have been
   * created by the user code. The PositionsManager does not own these pointers
   * so they will have to be destroyed by the user
   *
   * @param posList    A pointer to a PositionsContainer object. Cannot be null.
   * @param slippage   A pointer to a Slippage type object. It can be 0, in
   * which case no slippage will be used
   * @param commission A pointer to a Commission object. Can be 0, in which case
   * no commission will be used.
   *
   * @return A pointer to a Position type object
   * @see PositionsContainer
   * @see Slippage
   * @see Commission
   */
  CORE_API static PositionsManagerAbstrPtr create(PositionsContainer::PositionsContainerPtr posList, DateTime startTrades,
                                                DateTime endTrades, Slippage* slippage = 0, Commission* commission = 0);
  /**
   * Returns the total number of positions created, open or closed.
   *
   * Internally it calls the corresponding method of the associated
   * PositionsContainer object.
   *
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: how to enforce that pointers are not
   * null - maybe throw an exception</FONT> \endhtmlonly -->
   *
   * @return The total number of positions
   * @see PositionsPtrList
   */
  virtual size_t positionCount() const = 0;
  /**
   * Returns the total number of open positions created, open or closed.
   *
   * Internally it calls the corresponding method of the associated
   * PositionsContainer object.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: how to enforce that pointers are not
   * null - maybe throw an exception</FONT> \endhtmlonly -->
   *
   * @return The total number of open positions
   * @see PositionsPtrList
   */
  virtual size_t openPositionsCount() const = 0;
  /**
   * Registers a user defined OrderFilter
   *
   * This will enable order filtering for the current PositionsManager object.
   *
   * The order filter pointer can be 0, in which case no order filter will be
   * used.
   *
   * Notes:
   *  - There can only be one OrderFilter type object per PositionsManager. If
   * two or more are registered, only the last one will be  used
   *  - The OrderFilter type object is not destroyed by the destructor of this
   * class and the user has to manage its lifecycle and destroy when no longer
   * necessary.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: how to enforce that pointers are not
   * null - maybe throw an exception</FONT> \endhtmlonly -->
   *
   * @param orderFilter
   *               Pointer to a OrderFilter object - if 0, no order filter
   * @return The address of the previous order filter
   * @see OrderFilter
   */
  virtual OrderFilter* registerOrderFilter(OrderFilter* orderFilter) = 0;
  /**
   * Registers a user defined signalHandler
   *
   * Notes:
   *  - The user can register any number of signalHandler type objects, and all
   * of them will be notified in case of any signals, in the order in which they
   * were registered
   *  - The signalHandler type objects registered are not destroyed by the
   * destructor of this class and the user has to manage its lifecycle and
   * destroy when no longer necessary.
   *
   * Adding a null signalHandler* does not have any effect
   *
   * @param signalHandler Pointer to a signalHandler object.
   * @see signalHandler
   */
  virtual void registerSignalHandler(SignalHandler* signalHandler) = 0;
  /**
   * Regsiters multiple signal handlers in one call. All signal handlers to be
   * registered are passed as a vector of pointer to SignalHandler object.
   *
   * @param signalHandlers
   */
  virtual void registerSignalHandlers(
      std::vector<SignalHandler*> signalHandlers) = 0;

  /**@name Install automatic stops methods
   */
  //@{
  /**
   * Turns on and sets the number of bars for the time based exit strategy.
   *
   * The time based exit strategy will close a position (short or long) at
   * market after the specified number of bars
   *
   * Time based exit set this way is enabled for all open positions in the
   * current PositionsPtrList object.
   *
   * In order to perform the time based exit when it is activated, either of
   * applyTimeBased or applyAutoStops methods must be called on each bar.
   *
   * This method can be called multiple times, in which case only the last set
   * value is considered.
   *
   * @param bars   The number of bars after which an open position will be
   * closed
   * @see applyTimeBased
   * @see applyAutoStops
   */
  virtual void installTimeBasedExit(Index bars) = 0;
  virtual void installTimeBasedExitAtMarket(Index bars) = 0;
  virtual void installTimeBasedExitAtClose(Index bars) = 0;
  /**
   * Turns on and sets the level in % of the entry price for the break even stop
   * exit strategy.
   *
   * The break even stop strategy will close a position (short or long) if the
   * price reaches the entry price, after it had gone up (for long positions) or
   * down (for short positions) the specifie percentage. In other words, if a
   * position first shows a "level" percent profit, the break even stop will be
   * activated, and if the price goes back to the entry price the position will
   * be closed at stop, with the entry price set as stop price
   *
   * The break even stop exit strategy set this way is enabled for all open
   * positions in the current PositionsManager object.
   *
   * This method can be called multiple times, in which case only the last set
   * value is considered.
   *
   * In order to perform the break even stop exit when it is active, either of
   * applyBreakEvenStop or applyAutoStops methods need to be called on each bar.
   *
   * @param level  The price change level in % that will activate the break even
   * stop strategy
   *
   * @see applyBreakEvenStop
   * @see applyAutoStops
   */
  virtual void installBreakEvenStop(double level) = 0;
  virtual void installBreakEvenStopLong(double level) = 0;
  virtual void installBreakEvenStopShort(double level) = 0;
  /**
   * Turns on and sets the level in % of the entry price for the reverse break
   * even stop exit strategy.
   *
   * break even stop and
   *
   * The reverse break even stop strategy will close a position (short or long)
   * if the price reaches the entry price, after it had gone down (for long
   * positions) or up (for short positions) the specifie percentage. In other
   * words, if a position first shows a "level" percent loss, the reverse break
   * even stop will be activated, and if the price goes back to the entry level,
   * the position will be closed at limit, with the entry price set as limit
   * price
   *
   * The reverse break even stop exit strategy set this way is enabled for all
   * open positions in the current PositionsManager object.
   *
   * This method can be called multiple times, in which case only the last set
   * value is considered.
   *
   * In order to perform the reverse break even stop exit when it is active,
   * either of applyReverseBreakEvenStop or applyAutoStops methods need to be
   * called on each bar.
   *
   * @param level  The price change level in % that will activate the break even
   * stop strategy
   *
   * @see applyReverseBreakEvenStop
   * @see applyAutoStops
   */
  virtual void installReverseBreakEvenStop(double level) = 0;
  virtual void installReverseBreakEvenStopLong(double level) = 0;
  virtual void installReverseBreakEvenStopShort(double level) = 0;
  /**
   * Turns on and sets the level for the stop loss exit strategy.
   *
   * ?he stop loss strategy is the opposite of installStopLoss
   *
   * Stop loss exit strategy will close a position (long or short) if the loss
   * reaches "level" % of the entry price. The position will be closed at stop.
   *
   * Stop loss exit strategy set this way is enabled for all open positions in
   * the current PositionsManager object.
   *
   * In order to perform the stop loss exit when it is active, either of
   * applyStopLoss or applyAutoStops methods need to be called on each bar.
   *
   * This method can be called multiple times, in which case only the last set
   * value is considered.
   *
   * @param level  The stop loss level in percent of the entry price
   *
   * @see installProfitTarget
   * @see applyStopLoss
   * @see applyAutoStops
   */
  virtual void installStopLoss(double level) = 0;
  virtual void installStopLossLong(double level) = 0;
  virtual void installStopLossShort(double level) = 0;
  /**
   * Turns on and sets the profit target level for the profit target exit
   * strategy
   *
   * The Profit target exit strategy is the opposite of installStopLoss.
   * It will close a position when the price reaches "level" % of the entry
   * price. The position will be closed at limit.
   *
   * Profit target exit strategy set this way is enabled for all open positions
   * in the current PositionsManager object.
   *
   * In order to perform the profit target exit when it is active, either of
   * applyStopLoss or applyAutoStops methods need to be called on each bar.
   *
   * This method can be called multiple times, in which case only the last set
   * value is considered.
   *
   * @param level  Profit target level in percent of the entry price
   * @see installStopLoss
   * @see applyProfitTarget
   * @see applyAutoStops
   */
  virtual void installProfitTarget(double level) = 0;
  virtual void installProfitTargetLong(double level) = 0;
  virtual void installProfitTargetShort(double level) = 0;
  /**
   * Turns on and sets the level for the trailing stop exit strategy.
   *
   * The trigger is the percentage gain the position must to show before
   * activating or adjusting the trailing stop price. The level is the
   * percentage of the trigger price at which the stop price is set.
   *
   * The trailing stop is activated when the position reaches "trigger" percent
   * profit on the closing price of the current bar. Once activated, a loss stop
   * of "level" percent will be set on the position. In case the position
   * reaches another "trigger" percent profit above the last trigger point, the
   * stop loss price will be adjusted accordingly.
   *
   * This exit strategy set this way is enabled for all open positions in the
   * current PositionsManager object.
   *
   * In order to perform the trailing stop exit when it is active, either of
   * applyTrailingStop or applyAutoStops methods need to be called on each bar.
   *
   * This method can be called multiple times, in which case only the last set
   * value is considered.
   *
   * @param trigger trigger value
   * @param level   level value
   * @see applyTrailingStop
   * @see applyAutoStops
   */
  virtual void installTrailingStop(double trigger, double lossLevel) = 0;
  //@}
  /*  virtual void disableAllAutoStops() = 0;
    virtual void disableTimeBasedExit() = 0;
    virtual void disableBreakEvenStop() = 0;
    virtual void disableReverseBreakEvenStop() = 0;
    virtual void diableStopLoss() = 0;
    virtual void disableProfitTarget() = 0;
    virtual void disableTrailingStop() = 0;
  */
  /**@name Apply automatic stop methods
   *
   * All these methods call one of the position exit methods internally, such as
   * SellAtStop, CoverAtStop etc, and as a consequence, if trade filters have
   * been installed, they will be activated, so depending on the action of these
   * filters, the trade may actually not be triggered
   */
  //@{
  /**
   * Applies the time based exit strategy on a position at the current bar.
   *
   * For a more detailed description of the time based exit strategy refer to
   * installTimeBased
   *
   * @param bars     Reference to a Bars object
   * @param barIndex Current bar
   * @param pos      Position object on which to apply the time based exit
   * strategy
   * @exception BarIndexOutOfRangeException
   *                   thrown if the bar index is out of range
   * @see installTimeBased
   * @see applyAutoStops
   */
  virtual void applyTimeBased(Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyTimeBasedAtMarket( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyTimeBasedAtClose( Bars bars, size_t barIndex, Position pos) = 0;
  /**
   * Applies the stop loss exit strategy on a position at the current bar.
   *
   * For a detailed description of the stop loss exit strategy refer to
   * installStopLoss
   *
   *
   * @param bars     Reference to a Bars object
   * @param barIndex Current bar
   * @param pos      Position object on which to apply the stop lossexit
   * strategy
   * @exception BarIndexOutOfRangeException
   *                   thrown if the bar index is out of range
   * @see installStopLoss
   * @see applyAutoStops
   */
  virtual void applyStopLoss(Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyStopLossLong( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyStopLossShort( Bars bars, size_t barIndex, Position pos) = 0;
  /**
   * Applies the trailing stop exit strategy on a position at the current bar.
   *
   * For a detailed description of the trailing stop exit strategy refer to
   * installTrailingStop
   *
   *
   * @param bars     Reference to a Bars object
   * @param barIndex Current bar
   * @param pos      Position object on which to apply the trailing stop exit
   * strategy
   * @exception BarIndexOutOfRangeException
   *                   thrown if the bar index is out of range
   * @see installTrailingStop
   * @see applyAutoStops
   */
  virtual void applyTrailing(Bars, size_t barIndex, Position pos) = 0;
  /**
   * Applies the break even stop exit strategy on a positions at the current
   * bar.
   *
   * For a detailed description of the break even exit strategy refer to
   * installBreakEvenStop
   *
   *  Note: if the condition for exit is met, one of the position exit methods
   * will be called, so filters (if installed) will be applied to the trade and
   * the exit may not actually happen
   *
   * @param bars     Reference to a Bars object
   * @param barIndex Current bar
   * @param pos      Position object on which to apply the break even stop exit
   * strategy
   * @exception BarIndexOutOfRangeException
   *                   thrown if the bar index is out of range
   * @see installBreakEvenStop
   * @see applyAutoStops
   */
  virtual void applyBreakEvenStop( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyBreakEvenStopLong( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyBreakEvenStopShort( Bars bars, size_t barIndex, Position pos) = 0;
  /**
   * Applies the reverse break even stop exit strategy on a positions at the
   * current bar.
   *
   * For a detailed description of the reverse break even exit strategy refer to
   * installReverseBreakEvenStop
   *
   *  Note: if the condition for exit is met, one of the position exit methods
   * will be called, so filters (if installed) will be applied to the trade and
   * the exit may not actually happen
   *
   * @param bars     Reference to a Bars object
   * @param barIndex Current bar
   * @param pos      Position object on which to apply the reverse break even
   * stop exit strategy
   * @exception BarIndexOutOfRangeException
   *                   thrown if the bar index is out of range
   * @see installReverseBreakEvenStop
   * @see applyAutoStops
   */
  virtual void applyReverseBreakEvenStop( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyReverseBreakEvenStopLong( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyReverseBreakEvenStopShort( Bars bars, size_t barIndex, Position pos) = 0;
  /**
   * Applies the profit target stop exit strategy on a positions at the current
   * bar.
   *
   * For a detailed description of the profit target exit strategy refer to
   * installProfitTarget
   *
   *  Note: if the condition for exit is met, one of the position exit methods
   * will be called, so filters (if installed) will be applied to the trade and
   * the exit may not actually happen
   *
   * @param bars     Reference to a Bars object
   * @param barIndex Current bar
   * @param pos      Position object on which to apply the reverse profit target
   * exit strategy
   * @exception BarIndexOutOfRangeException
   *                   thrown if the bar index is out of range
   * @see installProfitTarget
   * @see applyAutoStops
   */
  virtual void applyProfitTarget( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyProfitTargetLong( Bars bars, size_t barIndex, Position pos) = 0;
  virtual void applyProfitTargetShort( Bars bars, size_t barIndex, Position pos) = 0;
  /**
   * Applies all the currently enabled auto stops to all the open positions on
   * the current bar.
   *
   * The order in which they are applied is:
   *  - time based
   *  - stop loss
   *  - trailing stop
   *  - break even stop
   *  - reverse break even stop
   *  - profit target
   *
   *  Note: if one of the condition for exit is met, one of the position exit
   * methods will be called, so filters (if installed) will be applied to the
   * trade and the exit may not happen
   *
   * @param bars     A Bars type object
   * @param barIndex Current bar
   * @exception BarIndexOutOfRangeException
   *                   thrown if the bar index is out of range
   * @see installTimeBasedExit
   * @see installStopLoss
   * @see installTrailingStop
   * @see installBreakEvenStop
   * @see installReverseBreakEvenStop
   * @see installProfitTarget
   * @see applyTimeBased
   * @see applyStopLoss
   * @see applyTrailing
   * @see applyBreakEvenStop
   * @see applyReverseBreakEvenStop
   * @see applyProfitTarget
   */
  virtual void applyAutoStops(Bars bars, size_t barIndex) = 0;
  //@}
  /**@name Long position entry methods
   */
  //@{
  /**
   * Opens a long position at market
   *
   * It is assumed that the order is placed before the open, and thus the
   * position entry price will be the open price of the entry bar.
   *
   * Before actually opening the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars     A reference to a Bars type object
   * @param barIndex bar index at which the position is to be open
   * @param shares   Number of shares
   * @param name     Name of position
   *  @return the position id if it was open, 0 if the position
   *          wasn't opened
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   */
  virtual PositionId buyAtMarket( Bars bars, size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  /**
   * Opens a long position at close
   *
   * It will open a long position with the entry price equal to the the entry
   * bar close price
   *
   * Before actually opening the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars     A reference to a Bars type object
   * @param barIndex bar index at which the position is to be open
   * @param shares   Number of shares
   * @param name     Name of position
   *  @return the position id if it was open, 0 if the position
   *          wasn't opened
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   */
  virtual PositionId buyAtClose( Bars bars, size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  /**
   * Opens a long position at stop
   *
   * It is assumed that the order is placed before the open on the specified
   * bar. The entry price will be either the entry bar open price (in case the
   * open price was higher than the stop price) or the stop price, if it is
   * lower than the high of the bar
   *
   * Before actually opening the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly   -->
   *
   * @param bars      A reference to a Bars type object
   * @param barIndex  bar index at which the position is to be open
   * @param stopPrice Stop price
   * @param shares    Number of shares
   * @param name      Name of position
   *  @return the position id if it was open, 0 if the position
   *          wasn't opened
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   */
  virtual PositionId buyAtStop( Bars bars, size_t barIndex, double stopPrice, size_t shares, const std::string& name,  bool applyPositionSizing = true) = 0;
  /**
   * Opens a long position at limit
   *
   * It is assumed that the order is placed before the open on the specified
   * bar. The entry price will be either the entry bar open price (in case the
   * open price was lower than the limit price) or the limit price, if it is
   * higher than the low of the bar.
   *
   * Before actually opening the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly   -->
   *
   * @param bars      A reference to a Bars type object
   * @param barIndex  bar index at which the position is to be open
   * @param limitPrice  Limit price
   * @param shares    Number of shares
   * @param name      Name of position
   *  @return the position id if it was open, 0 if the position
   *          wasn't opened
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   */

  virtual PositionId buyAtLimit( Bars bars, size_t barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  //@}
  /**@name Long positions exit methods
   */
  //@{
  /**
   * Closes a long position at market
   *
   * It is assumed that the order is placed before the open, and thus the
   * position exit price will be the open price of the entry bar.
   *
   * Before actually closing the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars     A reference to a Bars type object
   * @param barIndex bar index at which the position is to be closed
   * @param pos      Pointer to a Position to be closed
   * @param name     Name of position
   * @return true if position was closed, false if not
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @exception SellingShortPositionException
   *                   Thrown in case the position being sold is a short
   * position
   * @exception ClosingAlreadyClosedPositionException
   *                   Thrown in case the position being closed had already been
   * closed
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   * @see SellingShortPositionException
   * @see ClosingAlreadyClosedPositionException
   */
  virtual bool sellAtMarket( Bars bars, size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool sellAtMarket( Bars bars, size_t barIndex, PositionId pos, const std::string& name) = 0;
  /**
   * Closes a long position at close
   *
   * The position exit price is the close price at the current bar
   *
   * Before actually closing the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars     A reference to a Bars type object
   * @param barIndex bar index at which the position is to be closed
   * @param pos      Pointer to a Position to be closed
   * @param name     Name of position
   * @return true if position was closed, false if not
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @exception SellingShortPositionException
   *                   Thrown in case the position being sold is a short
   * position
   * @exception ClosingAlreadyClosedPositionException
   *                   Thrown in case the position being closed had already been
   * closed
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   * @see SellingShortPositionException
   * @see ClosingAlreadyClosedPositionException
   */
  virtual bool sellAtClose( Bars bars, size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool sellAtClose( Bars bars, size_t barIndex, PositionId pos, const std::string& name) = 0;
  /**
   * Closes a long position at stop
   *
   * It is assumed that the order is placed before the open on the specified
   * bar. The exit price will be either the exit bar open price (in case the
   * open price was lower than the stop price) or the stop price, if it is
   * higher than the low of the bar
   *
   * Before actually closing the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars      A reference to a Bars type object
   * @param barIndex  bar index at which the position is to be closed
   * @param pos       Pointer to a Position to be closed
   * @param stopPrice Stop price
   * @param name      Name of position
   * @return true if position was closed, false if not
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @exception SellingShortPositionException
   *                   Thrown in case the position being sold is a short
   * position
   * @exception ClosingAlreadyClosedPositionException
   *                   Thrown in case the position being closed had already been
   * closed
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   * @see SellingShortPositionException
   * @see ClosingAlreadyClosedPositionException
   */
  virtual bool sellAtStop( Bars bars, size_t barIndex, Position pos, double stopPrice, const std::string& name) = 0;
  virtual bool sellAtStop( Bars bars, size_t barIndex, PositionId pos, double stopPrice, const std::string& name) = 0;
  /**
   * Closes a long position at limit
   *
   * It is assumed that the order is placed before the open on the specified
   * bar. The exit price will be either the exit bar open price (in case the
   * open price was higher than the limit price) or the limit price, if it is
   * lower than the high of the bar
   *
   * Before actually closing the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars      A reference to a Bars type object
   * @param barIndex  bar index at which the position is to be closed
   * @param pos       Pointer to a Position to be closed
   * @param limitPrice  Limit price
   * @param name      Name of position
   * @return true if position was closed, false if not
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @exception SellingShortPositionException
   *                   Thrown in case the position being sold is a short
   * position
   * @exception ClosingAlreadyClosedPositionException
   *                   Thrown in case the position being closed had already been
   * closed
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   * @see SellingShortPositionException
   * @see ClosingAlreadyClosedPositionException
   */
  virtual bool sellAtLimit( Bars bars, size_t barIndex, Position pos, double limitPrice, const std::string& name) = 0;
  virtual bool sellAtLimit( Bars bars, size_t barIndex, PositionId pos, double limitPrice, const std::string& name) = 0;
  //@}
  /**@name Short positions entry methods
   */
  //@{
  /**
   * Opens a short position at market
   *
   * It is assumed that the order is placed before the open, and thus the
   * position entry price will be the open price at the entry bar.
   *
   * Before actually opening the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars     A reference to a Bars type object
   * @param barIndex bar index at which the position is to be open
   * @param shares   Number of shares
   * @param name     Name of position
   *  @return the position id if it was open, 0 if the position
   *          wasn't opened
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   */
  virtual PositionId shortAtMarket( Bars bars, size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  /**
   * Opens a short position at close
   *
   * It will open a long position with the entry price equal to the the entry
   * bar close price
   *
   * Before actually opening the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars     A reference to a Bars type object
   * @param barIndex bar index at which the position is to be open
   * @param shares   Number of shares
   * @param name     Name of position
   *  @return the position id if it was open, 0 if the position
   *          wasn't opened
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   */
  virtual PositionId shortAtClose( Bars bars, size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  /**
   * Opens a short position at stop
   *
   * It is assumed that the order is placed before the open on the specified
   * bar. The entry price will be either the entry bar open price (in case the
   * open price was lower than the stop price) or the stop price, if it is
   * higher than the low of the bar
   *
   * Before actually opening the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars      A reference to a Bars type object
   * @param barIndex  bar index at which the position is to be open
   * @param stopPrice Stop price
   * @param shares    Number of shares
   * @param name      Name of position
   *  @return the position id if it was open, 0 if the position
   *          wasn't opened
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   */
  virtual PositionId shortAtStop( Bars bars, size_t barIndex, double stopPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  /**
   * Opens a short position at limit
   *
   * It is assumed that the order is placed before the open on the specified
   * bar. The entry price will be either the entry bar open price (in case the
   * open price was higher than the limit price) or the limit price, if it is
   * lower than the high of the bar.
   *
   * Before actually opening the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars      A reference to a Bars type object
   * @param barIndex  bar index at which the position is to be open
   * @param limitPrice  Limit price
   * @param shares    Number of shares
   * @param name      Name of position
   *  @return the position id if it was open, 0 if the position
   *          wasn't opened
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   */
  virtual PositionId shortAtLimit( Bars bars, size_t barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) = 0;
  //@}
  /**@name Short positions exit methods
   */
  //@{
  /**
   * Closes a short position at market
   *
   * It is assumed that the order is placed before the open, and thus the
   * position exit price will be the open price of the entry bar.
   *
   * Before actually closing the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars     A reference to a Bars type object
   * @param barIndex bar index at which the position is to be closed
   * @param pos      Pointer to a Position to be closed
   * @param name     Name of position
   * @return true if position was closed, false if not
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @exception SellingShortPositionException
   *                   Thrown in case the position being covered is a long
   * position
   * @exception ClosingAlreadyClosedPositionException
   *                   Thrown in case the position being closed had already been
   * closed
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   * @see SellingShortPositionException
   * @see ClosingAlreadyClosedPositionException
   */
  virtual bool coverAtMarket( Bars bars, size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool coverAtMarket( Bars bars, size_t barIndex, PositionId pos, const std::string& name) = 0;
  /**
   * Closes a short position at close
   *
   * The position exit price is the close price at the current bar
   *
   * Before actually closing the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars     A reference to a Bars type object
   * @param barIndex bar index at which the position is to be open
   * @param pos      Pointer to a Position to be closed
   * @param name     Name of position
   * @return true if position was closed, false if not
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @exception SellingShortPositionException
   *                   Thrown in case the position being covered is a long
   * position
   * @exception ClosingAlreadyClosedPositionException
   *                   Thrown in case the position being closed had already been
   * closed
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   * @see SellingShortPositionException
   * @see ClosingAlreadyClosedPositionException
   */
  virtual bool coverAtClose( Bars bars, size_t barIndex, Position pos, const std::string& name) = 0;
  virtual bool coverAtClose( Bars bars, size_t barIndex, PositionId pos, const std::string& name) = 0;
  /**
   * Closes a short position at stop
   *
   * It is assumed that the order is placed before the open on the specified
   * bar. The exit price will be either the exit bar open price (in case the
   * open price was higher than the stop price) or the stop price, if it is
   * lower than the high of the bar
   *
   * Before actually closing the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars      A reference to a Bars type object
   * @param barIndex  bar index at which the position is to be closed
   * @param pos       Pointer to a Position to be closed
   * @param stopPrice Stop price
   * @param name      Name of position
   * @return true if position was closed, false if not
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @exception SellingShortPositionException
   *                   Thrown in case the position being covered is a long
   * position
   * @exception ClosingAlreadyClosedPositionException
   *                   Thrown in case the position being closed had already been
   * closed
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   * @see SellingShortPositionException
   * @see ClosingAlreadyClosedPositionException
   */
  virtual bool coverAtStop( Bars bars, size_t barIndex, Position pos, double stopPrice, const std::string& name) = 0;
  virtual bool coverAtStop( Bars bars, size_t barIndex, PositionId pos, double stopPrice, const std::string& name) = 0;
  /**
   * Closes a long position at limit
   *
   * It is assumed that the order is placed before the open on the specified
   * bar. The exit price will be either the exit bar open price (in case the
   * open price was lower than the limit price) or the limit price, if it is
   * higher than the low of the bar
   *
   * Before actually closing the position, the corresponding filter method in a
   * OrderFilter object is called, and in case this method returns false, the
   * position is not open.
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: explain how slippage affects
   * order</FONT> \endhtmlonly -->
   *
   * @param bars      A reference to a Bars type object
   * @param barIndex  bar index at which the position is to be closed
   * @param pos       Pointer to a Position to be closed
   * @param limitPrice  Limit price
   * @param name      Name of position
   * @return true if position was closed, false if not
   * @exception BarIndexOutOfRangeException
   *                   Thrown if the bar index points beyond the Bars object
   * range
   * @exception SellingShortPositionException
   *                   Thrown in case the position being covered is a long
   * position
   * @exception ClosingAlreadyClosedPositionException
   *                   Thrown in case the position being closed had already been
   * closed
   * @see OrderFilter
   * @see BarIndexOutOfRangeException
   * @see SellingShortPositionException
   * @see ClosingAlreadyClosedPositionException
   */
  virtual bool coverAtLimit( Bars bars, size_t barIndex, Position pos, double limitPrice, const std::string& name) = 0;
  virtual bool coverAtLimit( Bars bars, size_t barIndex, PositionId pos, double limitPrice, const std::string& name) = 0;

  virtual void closeAllAtMarket( Bars bars, size_t barIndex, const std::string& name) = 0;
  virtual void closeAllShortAtMarket( Bars bars, size_t barIndex, const std::string& name) = 0;
  virtual void closeAllLongAtMarket( Bars bars, size_t barIndex, const std::string& name) = 0;

  virtual void closeAllAtClose( Bars bars, size_t barIndex, const std::string& name) = 0;
  virtual void closeAllShortAtClose( Bars bars, size_t barIndex, const std::string& name) = 0;
  virtual void closeAllLongAtClose( Bars bars, size_t barIndex, const std::string& name) = 0;

  virtual void closeAllShortAtLimit( Bars bars, size_t barIndex, double price, const std::string& name) = 0;
  virtual void closeAllLongAtLimit( Bars bars, size_t barIndex, double price, const std::string& name) = 0;

  virtual void closeAllShortAtStop( Bars bars, size_t barIndex, double price, const std::string& name) = 0;
  virtual void closeAllLongAtStop( Bars bars, size_t barIndex, double price, const std::string& name) = 0;

  virtual void closeFirstLongAtMarketByShares( size_t shares, Bars bars, size_t barIndex, const std::string& name) = 0;
  virtual void closeFirstShortAtMarketByShares( size_t shares, Bars bars, size_t barIndex, const std::string& name) = 0;

  virtual tradery::Position getLastPosition() = 0;
  virtual tradery::Position getLastOpenPosition() = 0;
  virtual const tradery::Position getLastOpenPosition() const = 0;

  virtual OpenPositionsIterator getOpenPositionsIterator() = 0;

  /**
   * Clears all the positions, and resets the trade filter, commission and
   * slippage
   */
  virtual void reset() = 0;
  /**
   * Sets the slippage, commission and trade filter of the current list to the
   * slippage, commission and trade filter of another container.
   *
   * The two positions lists will then refer to the same slippage, commission,
   * and trade filter, so make sure no interference exists in a multithreaded
   * environmnent
   *
   * @param positions The source positions container from which to get the
   * slippage, commission and trade filter
   * @see Slippage
   * @see Commission
   * @see TradeFilter
   */
  virtual void init(PositionsManagerAbstr& positions) = 0;
};

class Positions : public PositionsManagerAbstr {
 private:
  PositionsManagerAbstr* _positions;

  bool validate() const {
    if (_positions) {
      return true;
    }
    else {
      assert(false);
      return false;
    }
  }

 public:
  Positions() {}

  Positions(PositionsManagerAbstr* positions) : _positions(positions) {}

  Positions(Positions& pos) : _positions(pos._positions) {}

  operator bool() const { return _positions != 0; }

  void setSystemName(const std::string& str) override {
    validate();
    _positions->setSystemName(str);
  }
  const std::string& systemName() const override {
    validate();
    return _positions->systemName();
  }

  void setSystemId(const std::string& str) override {
    validate();
    _positions->setSystemId(str);
  }
  const std::string& systemId() const override {
    validate();
    return _positions->systemId();
  }

  PositionId buyAtPrice(Bars bars, size_t barIndex, double price, size_t shares, const std::string& name, bool applyPositionSizing, PositionId id) override {
    validate();
    return _positions->buyAtPrice(bars, barIndex, price, shares, name, applyPositionSizing, id);
  }
  PositionId shortAtPrice( Bars bars, size_t barIndex, double price, size_t shares, const std::string& name, bool applyPositionSizing, PositionId id) override {
    validate();
    return _positions->shortAtPrice(bars, barIndex, price, shares, name, applyPositionSizing, id);
  }
  Position getPosition(PositionId id) override {
    validate();
    return _positions->getPosition(id);
  }

  void forEachOpenPosition(OpenPositionHandler& openPositionHandler, Bars bars, size_t bar) override {
    validate();
    _positions->forEachOpenPosition(openPositionHandler, bars, bar);
  }
  size_t positionCount() const override {
    validate();
    return _positions->positionCount();
  }
  size_t openPositionsCount() const override {
    validate();
    return _positions->openPositionsCount();
  }
  bool hasOpenPositions() const { return openPositionsCount() > 0; }
  OrderFilter* registerOrderFilter(OrderFilter* orderFilter) override {
    validate();
    return _positions->registerOrderFilter(orderFilter);
  }
  void registerSignalHandler(SignalHandler* signalHandler) override {
    validate();
    _positions->registerSignalHandler(signalHandler);
  }
  void registerSignalHandlers( std::vector<SignalHandler*> signalHandlers) override {
    validate();
    _positions->registerSignalHandlers(signalHandlers);
  }
  void installTimeBasedExit(Index bars) override {
    validate();
    _positions->installTimeBasedExit(bars);
  }
  void installTimeBasedExitAtMarket(Index bars) override {
    validate();
    _positions->installTimeBasedExitAtMarket(bars);
  }
  void installTimeBasedExitAtClose(Index bars) override {
    validate();
    _positions->installTimeBasedExitAtClose(bars);
  }
  void installBreakEvenStop(double level) override {
    validate();
    return _positions->installBreakEvenStop(level);
  }
  void installBreakEvenStopLong(double level) override {
    validate();
    return _positions->installBreakEvenStopLong(level);
  }
  void installBreakEvenStopShort(double level) override {
    validate();
    return _positions->installBreakEvenStopShort(level);
  }
  void installReverseBreakEvenStop(double level) override {
    validate();
    _positions->installReverseBreakEvenStop(level);
  }
  void installReverseBreakEvenStopLong(double level) override {
    validate();
    _positions->installReverseBreakEvenStopLong(level);
  }
  void installReverseBreakEvenStopShort(double level) override {
    validate();
    _positions->installReverseBreakEvenStopShort(level);
  }
  void installStopLoss(double level) override {
    validate();
    _positions->installStopLoss(level);
  }
  void installStopLossLong(double level) override {
    validate();
    _positions->installStopLossLong(level);
  }
  void installStopLossShort(double level) override {
    validate();
    _positions->installStopLossShort(level);
  }
  void installProfitTarget(double level) override {
    validate();
    _positions->installProfitTarget(level);
  }
  void installProfitTargetShort(double level) override {
    validate();
    _positions->installProfitTargetShort(level);
  }
  void installProfitTargetLong(double level) override {
    validate();
    _positions->installProfitTargetLong(level);
  }
  void installTrailingStop(double trigger, double lossLevel) override {
    validate();
    _positions->installTrailingStop(trigger, lossLevel);
  }
  void applyTimeBased(Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyTimeBased(bars, barIndex, pos);
  }
  void applyTimeBasedAtMarket( Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyTimeBasedAtMarket(bars, barIndex, pos);
  }
  void applyTimeBasedAtClose( Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyTimeBasedAtClose(bars, barIndex, pos);
  }
  void applyStopLoss(Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyStopLoss(bars, barIndex, pos);
  }
  void applyStopLossLong( Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyStopLossLong(bars, barIndex, pos);
  }
  void applyStopLossShort( Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyStopLossShort(bars, barIndex, pos);
  }
  void applyTrailing(Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyTrailing(bars, barIndex, pos);
  }
  void applyBreakEvenStop( Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyBreakEvenStop(bars, barIndex, pos);
  }
  void applyBreakEvenStopLong( Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyBreakEvenStopLong(bars, barIndex, pos);
  }
  void applyBreakEvenStopShort( Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyBreakEvenStopShort(bars, barIndex, pos);
  }
  void applyReverseBreakEvenStop( Bars bars, size_t barIndex, Position pos) override {
    validate();
    return _positions->applyReverseBreakEvenStop(bars, barIndex, pos);
  }
  void applyReverseBreakEvenStopLong( Bars bars, size_t barIndex, Position pos) override {
    validate();
    return _positions->applyReverseBreakEvenStopLong(bars, barIndex, pos);
  }
  void applyReverseBreakEvenStopShort( Bars bars, size_t barIndex, Position pos) override {
    validate();
    return _positions->applyReverseBreakEvenStopShort(bars, barIndex, pos);
  }
  void applyProfitTarget( Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyProfitTarget(bars, barIndex, pos);
  }
  void applyProfitTargetLong( Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyProfitTargetLong(bars, barIndex, pos);
  }
  void applyProfitTargetShort( Bars bars, size_t barIndex, Position pos) override {
    validate();
    _positions->applyProfitTargetShort(bars, barIndex, pos);
  }
  void applyAutoStops(Bars bars, size_t barIndex) override {
    validate();
    _positions->applyAutoStops(bars, barIndex);
  }
  PositionId buyAtMarket( Bars bars, size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) override {
    validate();
    return _positions->buyAtMarket(bars, barIndex, shares, name, applyPositionSizing);
  }
  PositionId buyAtClose( Bars bars, size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) override {
    validate();
    return _positions->buyAtClose(bars, barIndex, shares, name, applyPositionSizing);
  }
  PositionId buyAtStop( Bars bars, size_t barIndex, double stopPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) override {
    validate();
    return _positions->buyAtStop(bars, barIndex, stopPrice, shares, name, applyPositionSizing);
  }
  PositionId buyAtLimit( Bars bars, size_t barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) override {
    validate();
    return _positions->buyAtLimit(bars, barIndex, limitPrice, shares, name, applyPositionSizing);
  }
  bool sellAtMarket( Bars bars, size_t barIndex, Position pos, const std::string& name) override {
    validate();
    return _positions->sellAtMarket(bars, barIndex, pos, name);
  }
  bool sellAtMarket( Bars bars, size_t barIndex, PositionId pos, const std::string& name) override {
    validate();
    return _positions->sellAtMarket(bars, barIndex, pos, name);
  }
  bool sellAtClose( Bars bars, size_t barIndex, Position pos, const std::string& name) {
    validate();
    return _positions->sellAtClose(bars, barIndex, pos, name);
  }
  bool sellAtClose( Bars bars, size_t barIndex, PositionId pos, const std::string& name) override {
    validate();
    return _positions->sellAtClose(bars, barIndex, pos, name);
  }
  bool sellAtStop( Bars bars, size_t barIndex, Position pos, double stopPrice, const std::string& name) override {
    validate();
    return _positions->sellAtStop(bars, barIndex, pos, stopPrice, name);
  }
  bool sellAtStop( Bars bars, size_t barIndex, PositionId pos, double stopPrice, const std::string& name) override {
    validate();
    return _positions->sellAtStop(bars, barIndex, pos, stopPrice, name);
  }
  bool sellAtLimit( Bars bars, size_t barIndex, Position pos, double limitPrice, const std::string& name) override {
    validate();
    return _positions->sellAtLimit(bars, barIndex, pos, limitPrice, name);
  }
  bool sellAtLimit( Bars bars, size_t barIndex, PositionId pos, double limitPrice, const std::string& name) override {
    validate();
    return _positions->sellAtLimit(bars, barIndex, pos, limitPrice, name);
  }
  PositionId shortAtMarket( Bars bars, size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) override {
    validate();
    return _positions->shortAtMarket(bars, barIndex, shares, name, applyPositionSizing);
  }
  PositionId shortAtClose( Bars bars, size_t barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) override {
    validate();
    return _positions->shortAtClose(bars, barIndex, shares, name, applyPositionSizing);
  }
  PositionId shortAtStop( Bars bars, size_t barIndex, double stopPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) override {
    validate();
    return _positions->shortAtStop(bars, barIndex, stopPrice, shares, name, applyPositionSizing);
  }
  PositionId shortAtLimit( Bars bars, size_t barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) override {
    validate();
    return _positions->shortAtLimit(bars, barIndex, limitPrice, shares, name, applyPositionSizing);
  }
  bool coverAtMarket( Bars bars, size_t barIndex, Position pos, const std::string& name) override {
    validate();
    return _positions->coverAtMarket(bars, barIndex, pos, name);
  }
  bool coverAtMarket( Bars bars, size_t barIndex, PositionId pos, const std::string& name) override {
    validate();
    return _positions->coverAtMarket(bars, barIndex, pos, name);
  }
  bool coverAtClose( Bars bars, size_t barIndex, Position pos, const std::string& name) override {
    validate();
    return _positions->coverAtClose(bars, barIndex, pos, name);
  }
  bool coverAtClose( Bars bars, size_t barIndex, PositionId pos, const std::string& name) override {
    validate();
    return _positions->coverAtClose(bars, barIndex, pos, name);
  }
  bool coverAtStop( Bars bars, size_t barIndex, Position pos, double stopPrice, const std::string& name) override {
    validate();
    return _positions->coverAtStop(bars, barIndex, pos, stopPrice, name);
  }
  bool coverAtStop( Bars bars, size_t barIndex, PositionId pos, double stopPrice, const std::string& name) override {
    validate();
    return _positions->coverAtStop(bars, barIndex, pos, stopPrice, name);
  }
  bool coverAtLimit( Bars bars, size_t barIndex, Position pos, double limitPrice, const std::string& name) override {
    validate();
    return _positions->coverAtLimit(bars, barIndex, pos, limitPrice, name);
  }
  bool coverAtLimit( Bars bars, size_t barIndex, PositionId pos, double limitPrice, const std::string& name) override {
    validate();
    return _positions->coverAtLimit(bars, barIndex, pos, limitPrice, name);
  }
  void closeAllAtMarket( Bars bars, size_t barIndex, const std::string& name) override {
    validate();
    _positions->closeAllAtMarket(bars, barIndex, name);
  }
  void closeAllShortAtMarket( Bars bars, size_t barIndex, const std::string& name) override {
    validate();
    _positions->closeAllShortAtMarket(bars, barIndex, name);
  }
  void closeAllLongAtMarket( Bars bars, size_t barIndex, const std::string& name) override {
    validate();
    _positions->closeAllLongAtMarket(bars, barIndex, name);
  }
  void closeAllAtClose( Bars bars, size_t barIndex, const std::string& name) override {
    validate();
    _positions->closeAllAtClose(bars, barIndex, name);
  }
  void closeAllShortAtClose( Bars bars, size_t barIndex, const std::string& name) {
    validate();
    _positions->closeAllShortAtClose(bars, barIndex, name);
  }
  void closeAllLongAtClose( Bars bars, size_t barIndex, const std::string& name) {
    validate();
    _positions->closeAllLongAtClose(bars, barIndex, name);
  }
  void closeAllShortAtLimit( Bars bars, size_t barIndex, double limitPrice, const std::string& name) override {
    validate();
    _positions->closeAllShortAtLimit(bars, barIndex, limitPrice, name);
  }
  void closeAllLongAtLimit( Bars bars, size_t barIndex, double price, const std::string& name) override {
    validate();
    _positions->closeAllLongAtLimit(bars, barIndex, price, name);
  }
  void closeAllShortAtStop( Bars bars, size_t barIndex, double price, const std::string& name) override {
    validate();
    _positions->closeAllShortAtStop(bars, barIndex, price, name);
  }
  void closeAllLongAtStop( Bars bars, size_t barIndex, double price, const std::string& name) override {
    validate();
    _positions->closeAllLongAtStop(bars, barIndex, price, name);
  }
  void closeFirstLongAtMarketByShares( size_t shares, Bars bars, size_t barIndex, const std::string& name) override {
    validate();
    _positions->closeFirstLongAtMarketByShares(shares, bars, barIndex, name);
  }
  void closeFirstShortAtMarketByShares( size_t shares, Bars bars, size_t barIndex, const std::string& name) override {
    validate();
    _positions->closeFirstShortAtMarketByShares(shares, bars, barIndex, name);
  }
  void reset() override {
    validate();
    _positions->reset();
  }
  void init(PositionsManagerAbstr& positions) override {
    validate();
    _positions->init(positions);
  }

  tradery::Position getLastPosition() override {
    validate();
    return _positions->getLastPosition();
  }

  const tradery::Position getLastOpenPosition() const override {
    validate();
    return _positions->getLastOpenPosition();
  }

  tradery::Position getLastOpenPosition() override {
    validate();
    return _positions->getLastOpenPosition();
  }

  OpenPositionsIterator getOpenPositionsIterator() override {
    validate();
    return _positions->getOpenPositionsIterator();
  }
};

class ParametersException {};

using double_vector = std::vector<double>;
class Parameters : public double_vector {
 public:
  Parameters(size_t size) : double_vector(size) {}

  void setValue(size_t index, double value) {
    try {
      at(index) = value;
    }
    catch (const std::out_of_range&) {
      // index out of range
      // TODO: more informative exception
      throw ParametersException();
    }
  }

  double getValue(size_t index) const {
    try {
      return at(index);
    }
    catch (const std::out_of_range&) {
      // index out of range
      // TODO: more informative exception
      throw ParametersException();
    }
  }
};

class ExplicitTrades;

/**
 * Abstract class - base for all classes that can be run by the Scheduler
 *
 * The user will have to derive classes from Runnable or use existing classes
 * derived from Runnable and pass them to the Scheduler, in order to run them.
 *
 * A Runnable is not normally run directly by the user, but by a Scheduler. A
 * Scheduler object will run a system calling the method "run" once for each
 * symbol.
 *
 * A Runnable can be run multiple times on the same list of symbols, depending
 * on the value returned by the virtual method Runnable::again. This is
 * necessary for self-optimizing or adaptive systems. They will run multiple
 * times over the entire set of symbols, at each run changing the values of
 * certain internal parameters. At the end, the optimal set of parameter values
 * will be selected and a final run using these parameters these will be
 * performed.
 *
 * One example of a Runnable is the class BarSystem, which provides the basic
 * functionality for a bar based trading system. Another example is the class
 * Optimizer, which can be use to either optimize systems statically, or as a
 * basis for adaptive system that could be run in real trading.
 *
 * A Runnable has a name and a description
 *
 * @see Scheduler
 */
class Runnable : public PluginConfiguration {
 private:
  const Parameters* _parameters;
  const std::string _userString;

 public:
  Runnable(const Info& info, const std::string& userString)
      : PluginConfiguration(info), _parameters(0), _userString(userString) {}

  virtual ~Runnable() {}

  const std::string& getUserString() const { return _userString; }

  virtual void setParameters(const Parameters* parameters) {
    _parameters = parameters;
  }

  void exit(const std::string& exitMessage = std::string()) {
    throw ExitRunnableException(exitMessage);
  }

  virtual void run() = 0;
  /**
   * Called after the run method has been called on the last symbol in the
   * symbols list. Depending on the return, it will signal that the system
   * should be run again on all symbols (continue), or should stop.
   *
   * The default implementation stops the run after iterating once over all
   * symbols. So systems that do not implement it, run in normal simulation or
   * run mode. Normally it will be overriden by optimization or walk-forward
   * runnables.
   *
   * @return run again on all symbols if true, don't run again if false, the
   * default implementation signals no continue
   */
  virtual bool again() { return false; }
  /**
   * Called before running the system on the first symbol in the list. This is
   * the case on the first run, and also if the method again is called to
   * restart the execution process
   *
   * The default implementation doesn't do anything. It will typically be
   * overriden in runnable derived optimization or walk-forward classes.
   *
   * If the return value is false, the runnable is stopped immediately and it
   * will not run over any symbol
   *
   * @return true if the runnable is to be run normally, false to stop the
   * runnable from running on any symbol
   */
  virtual bool begin() { return true; }
  /**
   * Sets the default parameters for the current run on the current symbol
   * used internally only.
   *
   * @param bars      Pointer to a Bars object, containing the data to run on.
   * @param positions Pointer to a positions object, which will be set as the
   * default positions for the system.
   * @param os        Pointer to an OutputSink object, which will receive the
   * strings and control characters to be displayed
   *
   * @see OutputSink
   * @see OutputBuffer
   * @see DataCollection
   * @see PositionsManager
   */
  virtual void init(const DataCollection* bars, PositionsManagerAbstr* positions, chart::Chart* chartHandler, const tradery::ExplicitTrades* triggers) = 0;
  /**
   * Pure virtual function that has to be implemented by the user in Runnable
   * derived classes.
   *
   * Contain initialization code.
   *
   * This method is called by the scheduler every time the Runnable is about to
   * be run, on each symbol, after all the default parameters have been set and
   * before calling the actual run method, thus giving a chance to the user to
   * do its own init work.
   *
   * It also gives the user a chance to stop the system from running on the
   * curent symbol, if the return is false
   *
   * @param symbol The symbol on which the system is about to be run
   * @return true - continue with run, false - do not run the system on the
   * current symbol
   */
  virtual bool init(const std::string& symbol) = 0;
  /**
   * Pure virtual function - must to be implemented by the user in Runnable
   * derived classes.
   *
   * It is called by the scheduler for each run, on each symbol, after the run
   * method. Gives the user a chance to cleanup after the run.
   *
   * @see Scheduler
   */
  virtual void cleanup() = 0;
};

/**
 * A "smart" pointer to a Runnable
 *
 * @see Runnable
 */
using RunnablePtr = std::shared_ptr<Runnable>;

CORE_API DataRequester* getDataRequester();

}  // namespace tradery
   // end using namespace traderysimlib
